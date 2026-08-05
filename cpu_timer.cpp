//
// Created by tgian on 26. 8. 5..
//
#include "cpu_timer.hpp"
namespace async_mpmc::core {

    inline timer_ctx cpu_timer_start() {
        unsigned int dummy;
        _mm_lfence();
        return __rdtsc();
    }

    inline uint64_t cpu_timer_end(const timer_ctx ctx) {
        unsigned int dummy;
        return __rdtscp(&dummy) - ctx;
    }
}