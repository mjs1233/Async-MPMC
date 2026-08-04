//
// Created by tgian on 26. 8. 4..
//

#ifndef MCMS_ACTIONQUEUE_HPP
#define MCMS_ACTIONQUEUE_HPP
#include <queue>
#include "Action.hpp"


class ActionQueue {
public:
    ActionQueue();

    void push(std::unique_ptr<Action> action);
    std::optional<std::unique_ptr<Action>> pop();
private:
    std::queue<std::unique_ptr<Action>> m_actions;
};

#endif //MCMS_ACTIONQUEUE_HPP
