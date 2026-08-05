//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_EXECUTOR_HPP
#define MCMS_EXECUTOR_HPP
#include "PostSchedulerQueue.hpp"
#include <thread>
namespace async_mpmc {

    struct ExecutorConfig {
        size_t queue_size;

        uint32_t layer1_thres;
        uint32_t layer2_thres;

        uint32_t layer2_wait_ms;
    };

    class Executor {
    public:
        Executor(const ExecutorConfig& config, ActionStorage& action_storage);

        bool push(ActionHandle handle);

        void set_active(bool state);
        bool is_active() const;

    private:
        void run();
        uint32_t multi_layer_wait();
        size_t m_queue_size;
        PostSchedulerQueue m_post_scheduler_queue;
        std::jthread m_thread;
        std::atomic_bool m_active;
        uint32_t m_try_count = 0;

        uint32_t m_layer1_thres;
        uint32_t m_layer2_thres;
        uint32_t m_layer2_wait_us;
    };

}
#endif //MCMS_EXECUTOR_HPP
