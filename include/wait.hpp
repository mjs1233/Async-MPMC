//
// Created by tgian on 26. 8. 6..
//

#ifndef MCMS_WAIT_HPP
#define MCMS_WAIT_HPP
#include <cstdint>
#include <chrono>
#include <thread>

namespace async_mpmc::utils {
    struct MultiLayerWait {

        explicit MultiLayerWait(
            std::uint32_t m_layer1_thres,
            std::uint32_t m_layer2_thres,
            std::uint32_t m_layer2_wait_us
            );

        uint32_t operator()();
        uint32_t wait();

        uint32_t try_count() const;

        void reset();

    private:
        uint32_t m_try_count {};
        uint32_t m_layer1_thres;
        uint32_t m_layer2_thres;
        uint32_t m_layer2_wait_us;
    };


}
#endif //MCMS_WAIT_HPP
