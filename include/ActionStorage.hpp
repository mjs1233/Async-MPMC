//
// Created by tgian on 26. 8. 4..
//
#ifndef MCMS_ACTIONSTORAGE_HPP
#define MCMS_ACTIONSTORAGE_HPP
#include <atomic>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>
#include "Action.hpp"
namespace async_mpmc::scheduler {
    struct ActionHandle {
        uint64_t id;
        uint32_t index() const {
            return id & 0xFFFFFFFF;
        }
        uint32_t generation() const {
            return (id >> 32);
        }
    };
    class ActionStorage {
    private:
        struct Slot {
            Action action;
            std::atomic<uint32_t> next;
            std::atomic<uint32_t> generation;
        };
        struct alignas(8) TaggedHead {
            uint32_t generation;
            uint32_t index;
        };
    public:
        ActionStorage(std::size_t storage_size) : m_slots(storage_size) {
            if (storage_size == 0) {
                throw std::invalid_argument("ActionStorage requires at least one slot");
            }
            for (uint32_t i = 0; i < m_slots.size(); i++) {
                m_slots[i].generation.store(0);
                m_slots[i].next.store(i + 1);
            }
            m_slots.back().next.store(invalid_index);
        }
        std::optional<ActionHandle> register_action(Action&& action) {
            TaggedHead old_head = m_head.load(std::memory_order_acquire);
            TaggedHead new_head = {};
            while (true) {
                if (old_head.index == invalid_index) {
                    return std::nullopt;
                }
                new_head.index = m_slots[old_head.index].next.load(std::memory_order_acquire);
                new_head.generation = old_head.generation + 1;
                if (m_head.compare_exchange_weak(old_head, new_head,std::memory_order_release,std::memory_order_acquire)) {
                    break;
                }
            }
            Slot& new_slot = m_slots[old_head.index];
            new_slot.action = std::move(action);

            uint64_t id = static_cast<uint64_t>(old_head.index) + (static_cast<uint64_t>(new_slot.generation.load()) << 32);
            return ActionHandle{.id = id};
        }
        std::optional<Action> remove_action(ActionHandle action) {
            uint32_t idx = action.index();
            if (idx >= m_slots.size()) {
                return std::nullopt;
            }
            Slot& slot = m_slots[idx];
            uint32_t expected_gen = action.generation();
            if (!slot.generation.compare_exchange_strong(expected_gen, expected_gen + 1,
                                                         std::memory_order_acq_rel)) {
                return std::nullopt;
                                                         }
            std::optional<Action> result;
            result.emplace(std::move(slot.action));
            TaggedHead old_head = m_head.load(std::memory_order_acquire);
            uint32_t old_head_generation = 0;
            do {
                slot.next.store(old_head.index, std::memory_order_relaxed);
                old_head_generation = old_head.generation + 1;
            } while (!m_head.compare_exchange_weak(
                         old_head, TaggedHead{.generation = old_head_generation,.index = idx},
                         std::memory_order_release,
                         std::memory_order_acquire));
            return result;
        }
    private:
        std::vector<Slot> m_slots;
        std::atomic<TaggedHead> m_head{TaggedHead{.generation = 0, .index = 0}};
        static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();
    };
}
#endif //MCMS_ACTIONSTORAGE_HPP
