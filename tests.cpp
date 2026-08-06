#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "ActionStorage.hpp"
#include "PreSchedulerQueue.hpp"
#include "RoundRobinEngine.hpp"
#include "Scheduler.hpp"

namespace {

using namespace async_mpmc::scheduler;
using namespace std::chrono_literals;

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct NoopJob {
    void action() && {}
};

struct RecordJob {
    uint32_t value;
    std::vector<uint8_t>* seen;

    void action() && {
        if (value < seen->size()) {
            ++(*seen)[value];
        }
    }
};

struct SlowCountJob {
    std::atomic<uint32_t>* completed;

    void action() && {
        std::this_thread::sleep_for(1ms);
        completed->fetch_add(1, std::memory_order_relaxed);
    }
};

template <typename Predicate>
bool wait_until(Predicate&& predicate, std::chrono::milliseconds timeout) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!predicate()) {
        if (std::chrono::steady_clock::now() >= deadline) {
            return false;
        }
        std::this_thread::sleep_for(1ms);
    }
    return true;
}

void test_action_storage_generation() {
    ActionStorage storage{1};

    const auto first = storage.register_action(Action{NoopJob{}});
    require(first.has_value(), "first storage allocation failed");
    require(first->index() == 0 && first->generation() == 0, "first handle is invalid");
    require(storage.remove_action(*first).has_value(), "first storage removal failed");
    require(!storage.remove_action(*first).has_value(), "stale handle was accepted");

    const auto second = storage.register_action(Action{NoopJob{}});
    require(second.has_value(), "second storage allocation failed");
    require(second->index() == 0 && second->generation() == 1, "generation did not advance to one");
    require(storage.remove_action(*second).has_value(), "second storage removal failed");

    const auto third = storage.register_action(Action{NoopJob{}});
    require(third.has_value(), "third storage allocation failed");
    require(third->index() == 0 && third->generation() == 2, "generation did not advance to two");
    require(storage.remove_action(*third).has_value(), "third storage removal failed");
}

void test_pre_scheduler_queue_mpmc() {
    constexpr uint32_t producer_count = 4;
    constexpr uint32_t actions_per_producer = 300;
    constexpr uint32_t total_actions = producer_count * actions_per_producer;

    ActionStorage storage{128};
    PreSchedulerQueue queue{8, storage};
    std::vector<uint8_t> seen(total_actions, 0);
    std::atomic<uint32_t> pushed{0};
    std::atomic<bool> start{false};
    std::vector<std::thread> producers;
    producers.reserve(producer_count);

    for (uint32_t producer = 0; producer < producer_count; ++producer) {
        producers.emplace_back([&, producer] {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (uint32_t offset = 0; offset < actions_per_producer; ++offset) {
                const uint32_t value = producer * actions_per_producer + offset;
                while (!queue.push(Action{RecordJob{.value = value, .seen = &seen}})) {
                    std::this_thread::yield();
                }
                pushed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    start.store(true, std::memory_order_release);
    uint32_t popped = 0;
    bool storage_failure = false;
    while (popped != total_actions) {
        const auto handle = queue.pop();
        if (!handle.has_value()) {
            std::this_thread::yield();
            continue;
        }

        auto action = storage.remove_action(*handle);
        if (!action.has_value()) {
            storage_failure = true;
        } else {
            action->run();
        }
        ++popped;
    }

    for (auto& producer : producers) {
        producer.join();
    }

    require(!storage_failure, "pre-scheduler queue produced a stale or corrupted handle");
    require(pushed.load(std::memory_order_relaxed) == total_actions, "not every producer action was pushed");
    for (const auto count : seen) {
        require(count == 1, "pre-scheduler queue duplicated or lost an action");
    }
}

void test_scheduler_retries_full_executor_queue_and_shuts_down() {
    constexpr uint32_t total_actions = 64;
    std::atomic<uint32_t> completed{0};
    ActionStorage storage{128};

    {
        Scheduler<RoundRobinEngine> scheduler{
            1,
            RoundRobinEngine::config_type{},
            ExecutorConfig{
                .queue_size = 2,
                .layer1_thres = 5,
                .layer2_thres = 20,
                .layer2_wait_us = 50,
            },
            storage,
        };

        for (uint32_t i = 0; i < total_actions; ++i) {
            while (!scheduler.push(Action{SlowCountJob{.completed = &completed}})) {
                std::this_thread::yield();
            }
        }

        require(wait_until(
                    [&] { return completed.load(std::memory_order_relaxed) == total_actions; },
                    8s),
                "scheduler lost an action while an executor queue was full");

        scheduler.shutdown();
        require(!scheduler.is_active(), "scheduler did not stop");
    }

    require(completed.load(std::memory_order_relaxed) == total_actions,
            "scheduler shutdown changed the completed action count");
}

} // namespace

int main() {
    try {
        test_action_storage_generation();
        test_pre_scheduler_queue_mpmc();
        test_scheduler_retries_full_executor_queue_and_shuts_down();
        std::cout << "All scheduler queue tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}
