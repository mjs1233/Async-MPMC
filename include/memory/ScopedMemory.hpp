//
// Created by tgian on 26. 8. 7..
//

#ifndef MCMS_SCOPEDMEMORY_HPP
#define MCMS_SCOPEDMEMORY_HPP
#include <memory>
#include <cinttypes>

namespace async_mpmc::memory {

    class ScopedMemory {
    public:
        ScopedMemory(void* ptr, size_t size) : m_ptr{ptr}, m_size{size} {

        }

        ScopedMemory(ScopedMemory&& other) noexcept {
            m_ptr = other.m_ptr;
            m_size = other.m_size;
            other.m_ptr = nullptr;
            other.m_size = 0;
        }

        ScopedMemory& operator=(ScopedMemory&& other) noexcept {
            if (m_ptr != nullptr) {

            }

            m_ptr = other.m_ptr;
            m_size = other.m_size;
            other.m_ptr = nullptr;
            other.m_size = 0;
            return *this;
        }
        ScopedMemory(const ScopedMemory&) = delete;
        ScopedMemory& operator=(const ScopedMemory&) = delete;
        ~ScopedMemory();

        void* get() const {
            return m_ptr;
        }

        size_t get_size() const {
            return m_size;
        }

        void* operator->() const {
            return m_ptr;
        }

    private:
        void* m_ptr;
        size_t m_size;
    };

}
#endif //MCMS_SCOPEDMEMORY_HPP
