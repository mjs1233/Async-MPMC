//
// Created by tgian on 26. 8. 8..
//
#include "memory/SegregatedMemoryPool.hpp"

namespace async_mpmc::memory {

    SegregatedMemoryPool::SegregatedMemoryPool(const std::vector<SegregatedMemoryConfig>& memory_configs) :
    m_memory_count{memory_configs.size()} {

        void* raw_memory = ::operator new[](sizeof(__details::SegregatedMemory) * m_memory_count);
        m_segregated_memory = static_cast<__details::SegregatedMemory*>(raw_memory);
        uint32_t i = 0;
        for (const auto& config : memory_configs) {
            new (&m_segregated_memory[i]) __details::SegregatedMemory(config.alloc_size, config.alloc_count);
            ++i;
        }
    }

    SegregatedMemoryPool::~SegregatedMemoryPool() {
        if (m_segregated_memory) {
            for (size_t i = m_memory_count; i > 0; --i) {
                m_segregated_memory[i - 1].~SegregatedMemory();
            }
            ::operator delete[](m_segregated_memory);
        }
    }

    //
    std::optional<MemoryBlock> SegregatedMemoryPool::acquire(size_t size) {
        for (uint32_t i = 0; i < m_memory_count; ++i) {

            if (m_segregated_memory[i].get_alloc_size() != size) {
                continue;
            }

            void* ptr = nullptr;
            if (!m_segregated_memory[i].acquire(&ptr)) {
                return std::nullopt;
            }

            return MemoryBlock{ptr, size};

        }
        return std::nullopt;
    }

    void SegregatedMemoryPool::release(uint32_t size,void* ptr) {
        for (uint32_t i = 0; i < m_memory_count; ++i) {

            if (m_segregated_memory[i].get_alloc_size() != size) {
                continue;
            }

            m_segregated_memory[i].release(ptr);
        }
    }

    void SegregatedMemoryPool::init(const std::vector<SegregatedMemoryConfig>& memory_configs) {
        instance = new SegregatedMemoryPool(memory_configs);
    }

    SegregatedMemoryPool* SegregatedMemoryPool::get_instance() {
        return instance;
    }

    void SegregatedMemoryPool::terminate() {
        delete instance;
    }
}