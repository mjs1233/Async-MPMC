#include "PreSchedulerQueue.hpp"

namespace async_mpmc::scheduler {

    PreSchedulerQueue::PreSchedulerQueue(std::uint32_t queue_size, ActionStorage& action_storage)
        : m_queue_size(std::bit_ceil(queue_size))
        , m_mask(m_queue_size - 1)
        , m_action_storage(action_storage)
        , m_slots(m_queue_size)
    {
        for (size_t i = 0; i < m_queue_size; ++i) {
            m_slots[i].sequence.store(i, std::memory_order_relaxed);
        }
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
            uint64_t seq = slot.sequence.load(std::memory_order_acquire);
            intptr_t diff = static_cast<intptr_t>(seq) - static_cast<intptr_t>(pos);

            if (diff == 0) {
                if (m_enqueue_pos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                    slot.action_handle = handle.value();
                    slot.sequence.store(pos + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                m_action_storage.remove_action(handle.value());
                return false;
            } else {
                pos = m_enqueue_pos.load(std::memory_order_relaxed);
            }
        }
    }

    // single consumer only!
    std::optional<ActionHandle> PreSchedulerQueue::pop() {
        const uint64_t pos = m_dequeue_pos;
        Slot& slot = m_slots[pos & m_mask];

        const uint64_t seq = slot.sequence.load(std::memory_order_acquire);
        if (seq != pos + 1) {
            return std::nullopt;
        }

        // The acquire load above synchronizes with the producer's release store,
        // so action_handle is safe to read only after this point.
        ActionHandle handle = slot.action_handle;
        slot.sequence.store(pos + m_queue_size, std::memory_order_release);
        m_dequeue_pos = pos + 1;
        return handle;
    }

}
