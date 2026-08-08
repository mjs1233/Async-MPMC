//
// Created by tgian on 26. 8. 7..
//

#ifndef MCMS_SEGREGATEDMEMORYPOOL_HPP
#define MCMS_SEGREGATEDMEMORYPOOL_HPP

#include <vector>
#include <optional>
#include "SegregatedMemory.hpp"
#include "memory/MemoryBlock.hpp"


namespace async_mpmc::memory {

    struct SegregatedMemoryConfig {
        size_t alloc_size;
        size_t alloc_count;
    };

    class SegregatedMemoryPool {
    public:

        SegregatedMemoryPool(const std::vector<SegregatedMemoryConfig>& memory_configs);
        ~SegregatedMemoryPool();

        //
        std::optional<MemoryBlock> acquire(size_t size);
        void release(uint32_t size,void* ptr);

        static void init(const std::vector<SegregatedMemoryConfig>& memory_configs);
        static SegregatedMemoryPool* get_instance();
        static void terminate();

    private:
        static SegregatedMemoryPool* instance;
        size_t free_list_head = 0;
        __details::SegregatedMemory* m_segregated_memory = nullptr;
        size_t m_memory_count = 0;

    };

}

#endif //MCMS_SEGREGATEDMEMORYPOOL_HPP
