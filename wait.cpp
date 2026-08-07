//
// Created by tgian on 26. 8. 6..
//
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

#include "wait.hpp"

namespace async_mpmc::utils {

    namespace {
        static inline void cpu_relax() {
#if defined(__x86_64__) || defined(__i386__)
            _mm_pause();
#elif defined(__aarch64__) || defined(__arm__)
            asm volatile("yield" ::: "memory");
#elif defined(__riscv)
            asm volatile(".insn i 0x0F, 0, x0, x0, 0x010" ::: "memory");
#else
            asm volatile("" ::: "memory"); ///<--- complie in PowerPC...?
#endif
        }
    }

    MultiLayerWait::MultiLayerWait(std::uint32_t m_layer1_thres, std::uint32_t m_layer2_thres,std::uint32_t m_layer2_wait_us) :
            m_layer1_thres(m_layer1_thres),
            m_layer2_thres(m_layer2_thres),
            m_layer2_wait_us(m_layer2_wait_us) {}


    uint32_t MultiLayerWait::operator()(){
        return wait();
    }

    uint32_t MultiLayerWait::wait() {

        if (m_try_count < m_layer1_thres) {
            cpu_relax();
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