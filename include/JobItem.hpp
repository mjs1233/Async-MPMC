//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_JOBITEM_HPP
#define MCMS_JOBITEM_HPP

#include <concepts>
#include <cinttypes>

namespace async_mpmc::scheduler {
    static constexpr size_t MAX_JOB_SIZE = 128;


    template <typename T>
    concept job_item_trait = requires(T t) {
        { std::move(t).action() };
        { std::move(t).acquire() } -> std::same_as<bool>;
    }
    && !requires(T t)
    {
        { t.action() };
        { t.acquire() };
    }
    && (sizeof(T) <= MAX_JOB_SIZE);
}

#endif //MCMS_JOBITEM_HPP
