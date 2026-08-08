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

            explicit FreeTable(size_t alloc_count);
            uint64_t check_free_count() const;
            uint64_t acquire();
            void release(uint64_t index);

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
            explicit SegregatedMemory(size_t alloc_size, size_t alloc_count);

            size_t get_alloc_size() const;
            bool acquire(void** ptr);
            void release(void* ptr);

        private:
            size_t m_alloc_size = 0;
            size_t m_alloc_count = 0;
            FreeTable m_free_table;
            std::unique_ptr<std::byte[]> m_memory_ptr;
        };
    }
}
#endif //MCMS_SEGREGATEDMEMORY_HPP
