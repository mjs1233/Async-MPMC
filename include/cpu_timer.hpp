//
// Created by tgian on 26. 8. 5..
//

#ifndef MCMS_CPU_TIMER_HPP
#define MCMS_CPU_TIMER_HPP
#include <x86intrin.h>
#include <cinttypes>

namespace async_mpmc::core {
    using timer_ctx = uint64_t;

    timer_ctx cpu_timer_start();
    uint64_t cpu_timer_end(const timer_ctx ctx);
}

#endif //MCMS_CPU_TIMER_HPP
