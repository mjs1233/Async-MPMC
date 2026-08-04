//
// Created by tgian on 26. 8. 4..
//
#include "ActionQueue.hpp"

ActionQueue::ActionQueue() {

}

void ActionQueue::push(std::unique_ptr<Action> action) {

    m_actions.push(std::move(action));
}


std::optional<std::unique_ptr<Action>> ActionQueue::pop() {

    if (m_actions.empty()) {
        return std::nullopt;
    }
    std::unique_ptr<Action> action = std::move(m_actions.front());
    m_actions.pop();
    return action;
}