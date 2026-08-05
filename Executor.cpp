
#include <x86intrin.h>
#include "executor.hpp"
#include "cpu_timer.hpp"

namespace async_mpmc {
    Executor::Executor(const ExecutorConfig& config, ActionStorage& action_storage) :
    m_queue_size{config.queue_size},
    m_layer2_wait_us{config.layer2_wait_ms},
    m_layer1_thres{config.layer1_thres},
    m_layer2_thres{config.layer2_thres},
    m_post_scheduler_queue{config.queue_size, action_storage} {

        m_thread = std::jthread([this]() {
            run();
        });
    }

    bool Executor::push(ActionHandle handle) {
        return m_post_scheduler_queue.push(handle);
    }

    bool Executor::is_active() const {
        return m_active.load(std::memory_order_relaxed);
    }


    void Executor::set_active(bool state) {
        m_active.store(state);
    }

    void Executor::run() {
        while (m_active.load(std::memory_order_relaxed)) {

            auto optional_action = m_post_scheduler_queue.pop();
            if (!optional_action) {
                uint32_t result = multi_layer_wait();
                if (result == 2) {
                    ////// is queueing system dead? why no more work?
                }
                m_try_count++;
            }
            else {
                m_try_count = 0;
                core::timer_ctx timer_ctx_ = core::cpu_timer_start();
                optional_action.value().run();
                uint64_t run_time = core::cpu_timer_end(timer_ctx_);
            }
        }
    }

    uint32_t Executor::multi_layer_wait() {

        if (m_try_count < m_layer1_thres) {
            asm volatile("" ::: "memory");
            return 0;
        }
        else if (m_try_count < m_layer2_thres) {
            std::this_thread::yield();
            return 1;
        }
        else {
            std::this_thread::sleep_for(std::chrono::microseconds(m_layer2_wait_us));
            return 2;
        }
    }
}
