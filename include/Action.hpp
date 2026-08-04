//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_ACTION_HPP
#define MCMS_ACTION_HPP
#include <optional>
#include <utility>
#include <memory>

#include "JobItem.hpp"

/* type erasure for job item*/
class Action {
public:
    Action() = default;
    Action(const Action&) = delete;

    Action(Action&& other) noexcept {

        if (vtable_move != nullptr)
            vtable_move(m_job_item, other.m_job_item);

        vtable_action = other.vtable_action;
        vtable_move = other.vtable_move;
        vtable_dtor = other.vtable_dtor;

        other.vtable_action = nullptr;
        other.vtable_move = nullptr;
        other.vtable_dtor = nullptr;
    }

    Action& operator=(Action&& other) noexcept {
        if (vtable_move != nullptr)
            vtable_move(m_job_item, other.m_job_item);

        vtable_action = other.vtable_action;
        vtable_move = other.vtable_move;
        vtable_dtor = other.vtable_dtor;

        other.vtable_action = nullptr;
        other.vtable_move = nullptr;
        other.vtable_dtor = nullptr;
        return *this;
    }

    template <job_item_trait U>
    Action(U&& item) {
        using T = std::decay_t<U>;
        using ReturnType = decltype(std::declval<T>().action());


        new (m_job_item) T(std::forward<T>(item));

        vtable_action = [](void* ptr)-> std::optional<Action> {
            if constexpr (std::same_as<ReturnType, void>) {
                std::move(*static_cast<T*>(ptr)).action();
                return std::nullopt;
            }
            else {
                return std::move(*static_cast<T*>(ptr)).action();
            }
        };

        vtable_move = [](void* dst, void* src) {
            new (dst) T{std::move(*static_cast<T*>(src))};
        };

        vtable_dtor = [](void* ptr) -> void {
            return static_cast<T*>(ptr)->~T();
        };
    }

    ~Action() {
        if (vtable_dtor != nullptr) {
            vtable_dtor(m_job_item);
        }
    }

    void operator()() {
        run();
    }

    void run() {
        std::optional<Action> result = vtable_action(m_job_item);
        if (result.has_value()) {
            (*result)();
        }
    }



private:
    uint8_t m_job_item[MAX_JOB_SIZE] {};
    std::optional<Action>(*vtable_action)(void*) = nullptr;
    void (*vtable_move)(void*, void*) = nullptr;
    void (*vtable_dtor)(void*) = nullptr;

};


#endif //MCMS_ACTION_HPP
