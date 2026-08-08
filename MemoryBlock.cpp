#include "memory/MemoryBlock.hpp"
#include <memory/SegregatedMemoryPool.hpp>

namespace async_mpmc::memory {

    MemoryBlock::MemoryBlock(void* ptr, size_t size) : m_ptr{ptr}, m_size{size} {

    }

    MemoryBlock::MemoryBlock(MemoryBlock&& other) noexcept {
        m_ptr = other.m_ptr;
        m_size = other.m_size;
        other.m_ptr = nullptr;
        other.m_size = 0;
    }

    MemoryBlock& MemoryBlock::operator=(MemoryBlock&& other) noexcept {

        if (m_ptr != nullptr) {
            SegregatedMemoryPool::get_instance()->release(m_size,m_ptr);
        }

        m_ptr = other.m_ptr;
        m_size = other.m_size;
        other.m_ptr = nullptr;
        other.m_size = 0;
        return *this;
    }

    MemoryBlock::~MemoryBlock() {

        if (m_ptr != nullptr) {
            SegregatedMemoryPool::get_instance()->release(m_size,m_ptr);
        }
    }

    void* MemoryBlock::get() const {
        return m_ptr;
    }

    size_t MemoryBlock::get_size() const {
        return m_size;
    }

    void* MemoryBlock::operator->() const {
        return m_ptr;
    }

}