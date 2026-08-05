//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_SCHEDULER_HPP
#define MCMS_SCHEDULER_HPP
#include <queue>
#include <thread>
#include "PreSchedulerQueue.hpp"
#include "PostSchedulerQueue.hpp"
#include "Executor.hpp"
namespace async_mpmc {
    class Scheduler {
    public:
        Scheduler(size_t executor_size);

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        ~Scheduler();

        void set_active(bool active);
        bool is_active() const;



    private:
        void run();

        std::atomic<bool> m_active;
        std::jthread m_thread;

    };
}
#endif //MCMS_SCHEDULER_HPP
