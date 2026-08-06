#include <iostream>

#include "RoundRobinEngine.hpp"
#include <random>
#include "Scheduler.hpp"
#include <atomic>
#include <chrono>

#include "FileLoad.hpp"
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

    constexpr size_t executor_size = 1;
    constexpr size_t action_count = 2048;
    async_mpmc::scheduler::ActionStorage action_storage{ action_count };
    async_mpmc::scheduler::ActionStorage storage(16);
    auto h1 = storage.register_action(async_mpmc::scheduler::Action{});
    assert(h1.has_value());

    auto act1 = storage.remove_action(h1.value()); // 슬롯 0 반환
    assert(act1.has_value());

    auto h2 = storage.register_action(async_mpmc::scheduler::Action{}); // 슬롯 0 재사용
    assert(h2.has_value());
    assert(h2.value().generation() == 1);
    assert(h2.value().index() == 0);


    auto act2 = storage.remove_action(h2.value()); // 여기서 nullopt가 나는지 확인
    assert(act2.has_value());

    auto h3 = storage.register_action(async_mpmc::scheduler::Action{}); // 슬롯 0 재사용
    assert(h3.has_value());
    assert(h3.value().index() == 0);
    assert(h3.value().generation() == 2);

    auto act3 = storage.remove_action(h3.value()); // 여기서 nullopt가 나는지 확인
    assert(act3.has_value());

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

    std::this_thread::sleep_for(std::chrono::seconds(10));
    scheduler.shutdown();

    //TODO)
    // 1. load image from files
    // 2. convert image to pixel data
    // 3. apply random task
    // 4. do whatever that is
    // 5. save to file.
    return 0;
}
