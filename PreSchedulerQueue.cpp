#include "PreSchedulerQueue.hpp"

namespace async_mpmc::scheduler {

    PreSchedulerQueue::PreSchedulerQueue(std::uint32_t queue_size, ActionStorage& action_storage)
        : m_queue_size(std::bit_ceil(queue_size))
        , m_mask(m_queue_size - 1)
        , m_action_storage(action_storage)
        , m_slots(m_queue_size)
    {
    }

    bool PreSchedulerQueue::push(Action&& action) {
        std::optional<ActionHandle> handle =
            m_action_storage.register_action(std::move(action));

        if (!handle.has_value()) {
            return false;
        }

        std::uint64_t pos = m_enqueue_pos.load(std::memory_order_relaxed);

        for (;;) {
            Slot& slot = m_slots[pos & m_mask];

            if (slot.active.load(std::memory_order_acquire)) {
                m_action_storage.remove_action(handle.value());
                return false;
            }

            if (m_enqueue_pos.compare_exchange_weak(
                    pos, pos + 1,
                    std::memory_order_relaxed,
                    std::memory_order_relaxed)) {
                slot.action_handle = handle.value();
                slot.active.store(true, std::memory_order_release);
                return true;
            }
        }
    }

    // single consumer only!
    std::optional<ActionHandle> PreSchedulerQueue::pop() {
        Slot& slot = m_slots[m_dequeue_pos & m_mask];

        if (!slot.active.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        std::optional<ActionHandle> action_handle = slot.action_handle;

        slot.active.store(false, std::memory_order_release);

        m_dequeue_pos++;

        return action_handle;
    }

}