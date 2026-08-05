#include "PostSchedulerQueue.hpp"

namespace async_mpmc {

    PostSchedulerQueue::PostSchedulerQueue(size_t queue_size, ActionStorage& action_storage)
        : m_queue_size(std::bit_ceil(queue_size))
        , m_mask(m_queue_size - 1)
        , m_action_storage(action_storage)
        , m_slots(m_queue_size)
    {
    }

    // single producer only!
    bool PostSchedulerQueue::push(ActionHandle action_handle) {
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
    std::optional<Action> PostSchedulerQueue::pop() {
        Slot& slot = m_slots[m_dequeue_pos & m_mask];

        if (!slot.active.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        std::optional<Action> action = m_action_storage.remove_action(slot.action_handle);

        slot.active.store(false, std::memory_order_release);
        m_dequeue_pos++;

        return action;
    }

} // namespace async_mpmc