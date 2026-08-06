//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_EXECUTOR_HPP
#define MCMS_EXECUTOR_HPP
#include "PreSchedulerQueue.hpp"
#include "PostSchedulerQueue.hpp"
#include "wait.hpp"
#include <thread>
namespace async_mpmc::scheduler {

    struct ExecutorConfig {
        size_t queue_size;

        uint32_t layer1_thres;
        uint32_t layer2_thres;

        uint32_t layer2_wait_us;
    };

    class Executor {
    public:
        Executor(const ExecutorConfig& config,PreSchedulerQueue& pre_scheduler_queue, ActionStorage& action_storage);
        ~Executor();

        Executor(const Executor&) = delete;
        Executor& operator=(const Executor&) = delete;
        Executor(Executor&&) = delete;
        Executor& operator=(Executor&&) = delete;

        bool push(ActionHandle handle);

        void set_active(bool state);
        bool is_active() const;
        void shutdown();

    private:
        void run(std::stop_token stop_token);
        size_t m_queue_size {};
        utils::MultiLayerWait m_wait;

        PostSchedulerQueue m_post_scheduler_queue;
        PreSchedulerQueue& m_pre_scheduler_queue;
        std::jthread m_thread;
        std::atomic_bool m_active {};

    };

}
#endif //MCMS_EXECUTOR_HPP
