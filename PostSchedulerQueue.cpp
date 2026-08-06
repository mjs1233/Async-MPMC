#include "PostSchedulerQueue.hpp"

namespace async_mpmc {

    scheduler::PostSchedulerQueue::PostSchedulerQueue(size_t queue_size, ActionStorage& action_storage)
        : m_queue_size(std::bit_ceil(queue_size))
        , m_mask(m_queue_size - 1)
        , m_action_storage(action_storage)
        , m_slots(m_queue_size)
    {
    }

    // single producer only!
    bool scheduler::PostSchedulerQueue::push(ActionHandle action_handle) {
        Slot& slot = m_slots[m_enqueue_pos & m_mask];

        if (slot.active.load(std::memory_order_acquire)) {
            return false;
        }
        slot.action_handle = action_handle;
        slot.active.store(true, std::memory_order_release);
        m_enqueue_pos++;
        return true;
    }

    // single consumer only!
    std::optional<scheduler::Action> scheduler::PostSchedulerQueue::pop() {
        Slot& slot = m_slots[m_dequeue_pos & m_mask];

        if (!slot.active.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        std::optional<Action> action = m_action_storage.remove_action(slot.action_handle);

        if (!action.has_value()) {
            std::printf("[CRITICAL] Storage failed to return action for handle: %llu\n", (uint64_t)slot.action_handle.id);
        }

        slot.active.store(false, std::memory_order_release);
        m_dequeue_pos++;

        return action;
    }

} // namespace async_mpmc