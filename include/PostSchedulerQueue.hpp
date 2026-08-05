//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_POSTSCHEDULERQUEUE_HPP
#define MCMS_POSTSCHEDULERQUEUE_HPP

#include <atomic>
#include <optional>
#include <vector>
#include <bit>

#include "Action.hpp"
#include "ActionStorage.hpp"

namespace async_mpmc::scheduler {

    // single producer, single consumer 전용.
    class PostSchedulerQueue {
    public:
        explicit PostSchedulerQueue(size_t queue_size, ActionStorage& action_storage);

        // single producer only!
        bool push(ActionHandle action);

        // single consumer only!
        std::optional<Action> pop();

    private:
        struct Slot {
            std::atomic_bool active{false};
            ActionHandle action_handle{};
        };

        std::uint32_t m_queue_size {};
        std::uint32_t m_mask {};
        ActionStorage& m_action_storage;
        std::vector<Slot> m_slots {};

        alignas(64) std::uint64_t m_enqueue_pos{0};
        alignas(64) std::uint64_t m_dequeue_pos{0};
    };

}

#endif //MCMS_POSTSCHEDULERQUEUE_HPP
