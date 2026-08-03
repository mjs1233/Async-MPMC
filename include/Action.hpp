//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_ACTION_HPP
#define MCMS_ACTION_HPP
#include <utility>

#include "JobItem.hpp"

/* type erasure for job item*/
class Action {
public:
    template <job_item_trait T>
    Action(T&& item) {

        new (job_item) T(std::forward<T>(item));

        vtable_action = [&item]() -> Action {
            return std::move(item.action());
        };
        vtable_dtor = [&item]() -> void {
          item.~Action();
        };
    }

    ~Action() {
        if (vtable_dtor != nullptr) {
            vtable_dtor();
        }
    }


private:
    uint8_t job_item[MAX_JOB_SIZE] {};
    Action(*vtable_action)() = nullptr;
    void (*vtable_dtor)() = nullptr;







};


#endif //MCMS_ACTION_HPP
