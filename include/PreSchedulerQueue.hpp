#ifndef MCMS_PRESCHEDULERQUEUE_HPP
#define MCMS_PRESCHEDULERQUEUE_HPP

#include <atomic>
#include <optional>
#include <vector>
#include <bit>

#include "Action.hpp"
#include "ActionStorage.hpp"

namespace async_mpmc::scheduler {

    class PreSchedulerQueue {
    public:
        explicit PreSchedulerQueue(std::uint32_t queue_size, ActionStorage& action_storage);

        // 여러 producer가 동시에 호출 가능
        bool push(Action&& action);

        // single consumer only!
        std::optional<ActionHandle> pop();

    private:
        struct Slot {
            std::atomic_bool active{false};
            ActionHandle action_handle{};
        };

        std::uint32_t m_queue_size;
        std::uint32_t m_mask;
        ActionStorage& m_action_storage;
        std::vector<Slot> m_slots;

        alignas(64) std::atomic<std::uint64_t> m_enqueue_pos{0};
        alignas(64) std::uint64_t m_dequeue_pos{0};
    };

} // namespace async_mpmc

#endif //MCMS_PRESCHEDULERQUEUE_HPP