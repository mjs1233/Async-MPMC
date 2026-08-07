//
// Created by tgian on 26. 8. 7..
//

#ifndef MCMS_SEGREGATEDMEMORYPOOL_HPP
#define MCMS_SEGREGATEDMEMORYPOOL_HPP

#include <vector>
#include <optional>
#include "ScopedMemory.hpp"
#include "SegregatedMemory.hpp"
#include "SegregatedMemory.hpp"


namespace async_mpmc::memory {

    struct SegregatedMemoryConfig {
        size_t alloc_size;
        size_t alloc_count;
    };

    class SegregatedMemoryPool {
    public:
        SegregatedMemoryPool(const std::vector<SegregatedMemoryConfig>& memory_configs) :
        m_memory_count{memory_configs.size()} {

            void* raw_memory = ::operator new[](sizeof(__details::SegregatedMemory) * m_memory_count);
            m_segregated_memory = static_cast<__details::SegregatedMemory*>(raw_memory);
            uint32_t i = 0;
            for (const auto& config : memory_configs) {
                new (&m_segregated_memory[i]) __details::SegregatedMemory(config.alloc_size, config.alloc_count);
                ++i;
            }
        }

        ~SegregatedMemoryPool() {
            if (m_segregated_memory) {
                for (size_t i = m_memory_count; i > 0; --i) {
                    m_segregated_memory[i - 1].~SegregatedMemory();
                }
                ::operator delete[](m_segregated_memory);
            }
        }

        //
        std::optional<ScopedMemory> acquire(size_t size) {
            for (uint32_t i = 0; i < m_memory_count; ++i) {

                if (m_segregated_memory[i].get_alloc_size() != size) {
                    continue;
                }

                void* ptr = nullptr;
                if (!m_segregated_memory[i].acquire(&ptr)) {
                    return std::nullopt;
                }

                return ScopedMemory{ptr, size};

            }
            return std::nullopt;
        }

        void push_release(const ScopedMemory& memory) {
            m_free_list[free_list_head] = memory.get();
            free_list_head = (free_list_head + 1);
            assert(free_list_head == free_list_size);
        }

    private:
        static constexpr size_t free_list_size = 8;
        std::array<void*, free_list_size> m_free_list;
        size_t free_list_head = 0;
        __details::SegregatedMemory* m_segregated_memory = nullptr;
        size_t m_memory_count = 0;

    };

    void push_release(ScopedMemory& memory);
}

#endif //MCMS_SEGREGATEDMEMORYPOOL_HPP
