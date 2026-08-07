//
// Created by tgian on 26. 8. 7..
//

#ifndef MCMS_SEGREGATEDMEMORY_HPP
#define MCMS_SEGREGATEDMEMORY_HPP
#include <cinttypes>
#include <memory>
#include <atomic>
#include <array>
#include <cassert>
#include <limits>
#include <mutex>

#include "cpu_timer.hpp"
#include "wait.hpp"


namespace async_mpmc::memory {

    namespace __details {

        constexpr uint64_t FreeTableSVOLimit = 2048;
        constexpr uint64_t FreeTableInlineTableSize = (FreeTableSVOLimit + 63) / 64;

        struct FreeTable {
            FreeTable() = delete;

            explicit FreeTable(size_t alloc_count) {

                table_size = (alloc_count + 63) / 64;
                capacity = alloc_count;
                if (alloc_count > FreeTableSVOLimit) {
                    use_heap_table = true;
                    table_ptr = std::make_unique<std::atomic_uint64_t[]>(table_size);
                    for (uint64_t i = 0; i < table_size; ++i)
                       table_ptr[i].store(0, std::memory_order_relaxed);
                }
                std::atomic_uint64_t* target_table = use_heap_table ? table_ptr.get() : inline_table.data();

                //fill 1 to extra bits
                uint32_t remain = capacity % 64;
                if (remain != 0) {
                    uint64_t mask = ~((1ULL << remain) - 1);
                    target_table[table_size - 1].store(mask);
                }
                free_count = alloc_count;
            }

                uint64_t check_free_count() const {
                    return free_count;
                }

                uint64_t acquire() {

                    if (free_count.load(std::memory_order_relaxed) == 0) {
                        return invalid_index;
                    }

                    std::atomic_uint64_t* target_table = use_heap_table ? table_ptr.get() : inline_table.data();

                    for (uint32_t i = 0; i < table_size; ++i) {
                        uint64_t current = target_table[i].load(std::memory_order_relaxed);

                        while (current != 0xFFFFFFFFFFFFFFFFULL) {
                            uint64_t inverted = ~current;
                            uint32_t bit_offset = std::countr_zero(inverted);
                            uint64_t next = current | (1ULL << bit_offset);

                            if (target_table[i].compare_exchange_weak(current, next, std::memory_order_relaxed)) {

                                free_count.fetch_sub(1, std::memory_order_relaxed);

                                return (static_cast<uint64_t>(i) * 64) + bit_offset;
                            }
                        }
                    }

                    return invalid_index;
                }

                void release(uint64_t index) {
                    if (index >= capacity) {
                        return;
                    }

                    std::atomic_uint64_t* target_table = use_heap_table ? table_ptr.get() : inline_table.data();
                    uint64_t array_idx = index / 64;
                    uint64_t bit_offset = index % 64;

                    uint64_t mask = ~(1ULL << bit_offset);

                    uint64_t prev = target_table[array_idx].fetch_and(mask, std::memory_order_relaxed);

                    if ((prev & (1ULL << bit_offset)) != 0) {
                        free_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                static constexpr uint64_t invalid_index = std::numeric_limits<uint64_t>::max();

            private:
                std::unique_ptr<std::atomic_uint64_t[]> table_ptr = nullptr;
                uint64_t table_size = 0;
                uint64_t capacity;
                std::array<std::atomic_uint64_t, (FreeTableSVOLimit + 63) / 64> inline_table {};
                std::atomic_uint64_t free_count = 0;
                bool use_heap_table = false;



            };

        class SegregatedMemory {
        public:
            explicit SegregatedMemory(size_t alloc_size, size_t alloc_count) :
            m_alloc_size{alloc_size},
            m_alloc_count{alloc_count},
            m_free_table{alloc_count} {

                m_memory_ptr = std::make_unique<std::byte[]>(alloc_size * alloc_count);
                if (m_memory_ptr == nullptr) {
                    throw std::bad_alloc();
                }

            }

            size_t get_alloc_size() const {
                return m_alloc_size;
            }

            bool acquire(void** ptr) {
                uint64_t idx = m_free_table.acquire();
                if (idx == FreeTable::invalid_index) {
                    return false;
                }

                *ptr = m_memory_ptr.get() + idx * m_alloc_size;
                return true;
            }

            void batch_release(void* ptr[], size_t count) {

                for (size_t i = 0; i < count; i++) {
                    uintptr_t offset = reinterpret_cast<uintptr_t>(ptr[i]) - reinterpret_cast<uintptr_t>(m_memory_ptr.get());
                    assert(offset % m_alloc_size == 0);
                    m_free_table.release(offset / m_alloc_size);
                }
            }

        private:
            size_t m_alloc_size = 0;
            size_t m_alloc_count = 0;
            FreeTable m_free_table;
            std::unique_ptr<std::byte[]> m_memory_ptr;
        };
    }
}
#endif //MCMS_SEGREGATEDMEMORY_HPP
