//
// Created by tgian on 26. 8. 6..
//

#include "wait.hpp"
namespace async_mpmc::scheduler::utils {

    MultiLayerWait::MultiLayerWait(std::uint32_t m_layer1_thres, std::uint32_t m_layer2_thres,std::uint32_t m_layer2_wait_us) :
            m_layer1_thres(m_layer1_thres),
            m_layer2_thres(m_layer2_thres),
            m_layer2_wait_us(m_layer2_wait_us) {}


    uint32_t MultiLayerWait::operator()(){
        return wait();
    }

    uint32_t MultiLayerWait::wait() {

        if (m_try_count < m_layer1_thres) {
            asm volatile("" ::: "memory");
        }
        else if (m_try_count < m_layer2_thres) {
            std::this_thread::yield();
        }
        else {
            std::this_thread::sleep_for(std::chrono::microseconds(m_layer2_wait_us));
        }

        return ++m_try_count;
    }


    uint32_t MultiLayerWait::try_count() const {
        return m_try_count;
    }

    void MultiLayerWait::reset() {
        m_try_count = 0;
    }
}