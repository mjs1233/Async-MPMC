
#include <x86intrin.h>
#include "executor.hpp"
#include "cpu_timer.hpp"
#include "PreSchedulerQueue.hpp"

namespace async_mpmc::scheduler {
    Executor::Executor(const ExecutorConfig& config,PreSchedulerQueue& pre_scheduler_queue, ActionStorage& action_storage) :
    m_queue_size{config.queue_size},
    m_wait{config.layer1_thres,config.layer2_thres, config.layer2_wait_us},
    m_pre_scheduler_queue{pre_scheduler_queue},
    m_post_scheduler_queue{config.queue_size, action_storage} {

        m_active.store(true, std::memory_order_acquire);

        m_thread = std::jthread([this](std::stop_token stop_token) {
            run(stop_token);
        });
    }

    Executor::~Executor() {
        shutdown();
    }

    bool Executor::push(ActionHandle handle) {
        return m_post_scheduler_queue.push(handle);
    }

    bool Executor::is_active() const {
        return m_active.load(std::memory_order_relaxed);
    }


    void Executor::set_active(bool state) {
        m_active.store(state, std::memory_order_release);
        if (!state) {
            m_thread.request_stop();
        }
    }

    void Executor::shutdown() {
        m_active.store(false, std::memory_order_release);
        m_thread.request_stop();
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void Executor::run(std::stop_token stop_token) {
        while (!stop_token.stop_requested() && m_active.load(std::memory_order_acquire)) {

            auto optional_action = m_post_scheduler_queue.pop();
            if (!optional_action) {
                m_wait();
            }
            else {
                m_wait.reset();
                core::timer_ctx timer_ctx_ = core::cpu_timer_start();
                auto next_action = optional_action.value().run();

                ///flush free memory w/ spin lock

                if (next_action.has_value() && !stop_token.stop_requested() &&
                    m_active.load(std::memory_order_acquire)) {
                    m_pre_scheduler_queue.push(std::move(next_action.value()));
                }
                uint64_t run_time = core::cpu_timer_end(timer_ctx_);
            }
        }
    }


}
