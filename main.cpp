#include <iostream>

#include "RoundRobinEngine.hpp"
#include <random>
#include "Scheduler.hpp"
#include <atomic>
#include <chrono>
#include <future>

#include "FileLoad.hpp"
#include "memory/MemoryBlock.hpp"
#include "memory/SegregatedMemoryPool.hpp"
inline static std::atomic<uint32_t> next_id{};
struct random_delay_task {
    uint32_t delay;
    uint32_t id;

    random_delay_task() {
        thread_local static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<uint32_t> dist(0, 1000);

        delay = dist(gen);
        id = next_id.fetch_add(1, std::memory_order_relaxed) + 1;
        printf("[id : %d] generated\n", id);
    }

    void action() && {
        printf("[id : %d]  dly : %d\n", id, delay);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        printf("[id : %d]  dly : %d\n", id, delay);
        return;
    }
};



#include <iostream>
#include <filesystem>
#include <string>
#include <algorithm>

// 앞서 구현한 async_mpmc 헤더/클래스들이 포함되어 있다고 가정합니다.

void schedule_images_in_directory(async_mpmc::scheduler::Scheduler<async_mpmc::scheduler::RoundRobinEngine>& scheduler, const std::filesystem::path& dir_path) {
    namespace fs = std::filesystem;

    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cerr << "[Error] Invalid directory path: " << dir_path << std::endl;
        return;
    }

    uint32_t scheduled_count = 0;

    // 디렉터리 내의 모든 파일을 재귀적으로 탐색 (하위 디렉터리 제외를 원하면 directory_iterator 사용)
    for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        // 확장자 확인 (대소문자 구분을 위해 소문자로 변환)
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        if (ext == ".jpg" || ext == ".jpeg" || ext == ".png") {
            // ImageLoadJob 생성 및 스케줄러 push
            //std::printf("[image] push %s", entry.path().string().c_str());
            if (!scheduler.push(async_mpmc::ImageLoadJob{ entry.path() })) {
                std::printf("[Push Fail] Failed to schedule image: %s\n", entry.path().string().c_str());
            } else {
                scheduled_count++;
            }
        }
    }

    std::printf("[Scheduler] Successfully scheduled %u image jobs from %s\n",
                scheduled_count, dir_path.string().c_str());
}

int main() {

    std::vector<async_mpmc::memory::SegregatedMemoryConfig> memory_configs;
    memory_configs.emplace_back(async_mpmc::memory::SegregatedMemoryConfig{ .alloc_size = 128, .alloc_count = 256 });
    memory_configs.emplace_back(async_mpmc::memory::SegregatedMemoryConfig{ .alloc_size = 512, .alloc_count = 256 });
    memory_configs.emplace_back(async_mpmc::memory::SegregatedMemoryConfig{ .alloc_size = 1024, .alloc_count = 256 });
    memory_configs.emplace_back(async_mpmc::memory::SegregatedMemoryConfig{ .alloc_size = 65536, .alloc_count = 16 });
    async_mpmc::memory::SegregatedMemoryPool::init(memory_configs);
    async_mpmc::memory::SegregatedMemoryPool* pool = async_mpmc::memory::SegregatedMemoryPool::get_instance();

    size_t mem_size[4] = {128, 512, 1024, 65536};
    std::vector<async_mpmc::memory::MemoryBlock> scoped_mem;

    for (size_t i = 0; i < 100000; i++) {

        uint32_t target_size = mem_size[std::rand() % 4];
        uint8_t rands[5];
        for (uint32_t i = 0; i < 5; ++i) {
            rands[i] = std::rand() % 256;
        }
        async_mpmc::core::timer_ctx timer =
            async_mpmc::core::cpu_timer_start();
        auto m = pool->acquire(target_size);
        if (!m) {
            uint64_t clock_count = async_mpmc::core::cpu_timer_end(timer);
            std::printf("fail time %lld", clock_count);
            std::atomic_thread_fence(std::memory_order_acq_rel);
            continue;
        }


        uint64_t clock_count = async_mpmc::core::cpu_timer_end(timer);
    }




    constexpr size_t executor_size = 5;
    constexpr size_t action_count = 2048;
    async_mpmc::scheduler::ActionStorage action_storage{ action_count };
    async_mpmc::scheduler::Scheduler<async_mpmc::scheduler::RoundRobinEngine> scheduler {
        executor_size,
        async_mpmc::scheduler::RoundRobinEngine::config_type{},
        async_mpmc::scheduler::ExecutorConfig{
            .queue_size = 16,
            .layer1_thres = 100,
            .layer2_thres = 1000,
            .layer2_wait_us = 1000
        },
        action_storage
    };


    std::filesystem::path input_dir = "../res";

    schedule_images_in_directory(scheduler, input_dir);

    std::this_thread::sleep_for(std::chrono::seconds(150));
    scheduler.shutdown();

    //TODO)
    // 1. load image from files
    // 2. convert image to pixel data
    // 3. apply random task
    // 4. do whatever that is
    // 5. save to file.
    return 0;
}
