//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_SCHEDULER_HPP
#define MCMS_SCHEDULER_HPP
#include <queue>
#include <stdexcept>
#include <thread>
#include "PreSchedulerQueue.hpp"
#include "PostSchedulerQueue.hpp"
#include "Executor.hpp"
#include "wait.hpp"
namespace async_mpmc::scheduler {


    template <typename engine>
    class Scheduler {
    public:
        template <typename EngineConfig>
        requires std::same_as<typename engine::config_type, EngineConfig>
        Scheduler(size_t executor_size,const EngineConfig& engine_config_,const ExecutorConfig& executor_config ,ActionStorage& action_storage) :
        m_engine{engine_config_},
        m_wait {100,1000,1000},
        m_pre_scheduler_queue {1024,action_storage} {
            if (executor_size == 0) {
                throw std::invalid_argument("Scheduler requires at least one executor");
            }

            for (size_t i = 0; i < executor_size; ++i) {
                m_executors.emplace_back(executor_config, m_pre_scheduler_queue, action_storage);
            }
            m_active.store(true, std::memory_order_release);

            m_thread = std::jthread([this](std::stop_token stop_token) {
                run(stop_token);
            });
        }

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        ~Scheduler() {
            shutdown();
        }

        void set_active(bool active) {
            m_active.store(active, std::memory_order_release);
            if (!active) {
                m_thread.request_stop();
            }
        }

        bool is_active() const {
            return m_active.load(std::memory_order_acquire);
        }

        void shutdown() {
            m_active.store(false, std::memory_order_release);
            m_thread.request_stop();
            if (m_thread.joinable()) {
                m_thread.join();
            }

            // Executors can publish a continuation to m_pre_scheduler_queue while
            // finishing their current action, so they must be joined before the
            // queue is destroyed during member teardown.
            for (auto& executor : m_executors) {
                executor.shutdown();
            }
        }

        bool push(Action&& action) {
            if (!m_active.load(std::memory_order_acquire)) {
                return false;
            }
            return m_pre_scheduler_queue.push(std::move(action));
        }


    private:
        void run(std::stop_token stop_token) {

            m_engine.set_executors(m_executors);
            std::optional<ActionHandle> pending;

            while (!stop_token.stop_requested() && m_active.load(std::memory_order_acquire)) {
                if (!pending.has_value()) {
                    pending = m_pre_scheduler_queue.pop();
                    if (!pending.has_value()) {
                        m_wait();
                        continue;
                    }
                    m_wait.reset();
                }

                const size_t executor_index = m_engine.choose_executor();
                if (m_executors[executor_index].push(pending.value())) {
                    m_engine.report_push_result(true);
                    pending.reset();
                    m_wait.reset();
                } else {
                    m_engine.report_push_result(false);
                    m_wait();
                }
            }
        }
        engine m_engine;
        std::atomic<bool> m_active{false};
        std::jthread m_thread;
        std::deque<Executor> m_executors;
        utils::MultiLayerWait m_wait;
        PreSchedulerQueue m_pre_scheduler_queue;

    };
}
#endif //MCMS_SCHEDULER_HPP
