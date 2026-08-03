//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_JOBITEM_HPP
#define MCMS_JOBITEM_HPP

#include <concepts>
#include <cinttypes>

static constexpr size_t MAX_JOB_SIZE = 64;


template <typename T>
concept job_item_trait = requires(T t) {
    { std::move(t).action() };
}
&& !requires(T t)
{
    { t.action() };
}
&& (sizeof(T) < 64);



#endif //MCMS_JOBITEM_HPP
