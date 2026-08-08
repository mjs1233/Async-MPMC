//
// Created by tgian on 26. 8. 7..
//

#ifndef MCMS_MEMORYBLOCK_HPP
#define MCMS_MEMORYBLOCK_HPP
#include <memory>
#include <cinttypes>

namespace async_mpmc::memory {

    class MemoryBlock {
    public:
        MemoryBlock(void* ptr, size_t size);
        MemoryBlock(MemoryBlock&& other) noexcept;

        MemoryBlock& operator=(MemoryBlock&& other) noexcept;
        MemoryBlock(const MemoryBlock&) = delete;
        MemoryBlock& operator=(const MemoryBlock&) = delete;
        ~MemoryBlock();

        void* get() const;
        size_t get_size() const;
        void* operator->() const;

    private:
        void* m_ptr;
        size_t m_size;
    };

}
#endif //MCMS_MEMORYBLOCK_HPP
