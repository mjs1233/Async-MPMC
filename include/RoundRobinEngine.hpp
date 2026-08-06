//
// Created by tgian on 26. 8. 6..
//

#ifndef MCMS_ROUNDROBINENGINE_HPP
#define MCMS_ROUNDROBINENGINE_HPP
#include <queue>

#include "Executor.hpp"

namespace async_mpmc::scheduler {

    namespace {

    }

    struct RoundRobinEngineConfig {};

    class RoundRobinEngine {
    public:
        RoundRobinEngine(const RoundRobinEngineConfig& config) {

        }

        void set_executors(const std::deque<Executor>& executors) {
            m_executor_count = executors.size();
            m_fail_count.resize(m_executor_count);
        }

        size_t choose_executor() {
            m_executor_index = (m_executor_index + 1) % m_executor_count;
            return m_executor_index;
        }

        void report_push_result(bool success) {
            if (!success) {
                m_fail_count[m_executor_index]++;
            }
        }

    using config_type = RoundRobinEngineConfig;

    private:
        size_t m_executor_index {};
        size_t m_executor_count;
        std::vector<uint32_t> m_fail_count;
    };

}
#endif //MCMS_ROUNDROBINENGINE_HPP
