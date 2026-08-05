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
    template <typename engine>
    class Scheduler {
    public:
        template <typename engine_config>
        requires std::same_as<typename engine::config_type, engine_config>
        Scheduler(size_t executor_size, engine_config engine_config_, ActionStorage& action_storage) :
        m_engine{engine_config_} {

            for (size_t i = 0; i < executor_size; ++i) {
                m_executors.emplace_back(engine_config_, action_storage);
            }
        }

        Scheduler(const Scheduler&) = delete;
        Scheduler& operator=(const Scheduler&) = delete;

        ~Scheduler();

        void set_active(bool active);
        bool is_active() const;



    private:
        void run() {
            uint16_t cost = m_engine.calc_cost();

        }
        engine m_engine;
        std::atomic<bool> m_active;
        std::jthread m_thread;
        std::vector<Executor> m_executors;

    };
}
#endif //MCMS_SCHEDULER_HPP
