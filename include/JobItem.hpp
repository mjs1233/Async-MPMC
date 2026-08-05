//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_JOBITEM_HPP
#define MCMS_JOBITEM_HPP

#include <concepts>
#include <cinttypes>

#include "Action.hpp"
namespace async_mpmc {
    static constexpr size_t MAX_JOB_SIZE = 64;


    template <typename T>
    concept job_item_trait = requires(T t) {
        { std::move(t).action() };
    }
    && !requires(T t)
    {
        { t.action() };
    }
    && (sizeof(T) < MAX_JOB_SIZE);
}

#endif //MCMS_JOBITEM_HPP
