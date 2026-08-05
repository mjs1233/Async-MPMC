//
// Created by tgian on 26. 8. 6..
//

#ifndef MCMS_ACTIONCOST_HPP
#define MCMS_ACTIONCOST_HPP
#include <cstdint>

namespace async_mpmc::core {

    enum ActionType : uint8_t {
        pure_compute,
        blocking_compute, //i.e large memory alloc in memory pool
        file_io,
        socket_io
    };

    enum ActionFlags : uint8_t {
        has_child_action = 1 << 0,
        heavy_computing_per_unit = 1 << 1
        //...
    };

    struct ActionCost {
        uint32_t size;
        ActionType type;
        uint8_t flags;
    };

}

#endif //MCMS_ACTIONCOST_HPP
