#include <iostream>
#include <vector>
#include <filesystem>
#include <utility>
#include <cstdint>
#include <algorithm>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb/stb_image_write.h"

namespace async_mpmc {

    struct ImageWriteJob {
        std::vector<uint8_t> pixel_data;
        int width{0};
        int height{0};
        int channels{0};
        std::filesystem::path target_path;

        ImageWriteJob(std::vector<uint8_t>&& data, int w, int h, int ch, std::filesystem::path path)
            : pixel_data(std::move(data)), width(w), height(h), channels(ch), target_path(std::move(path)) {}

        void action() && {
            if (pixel_data.empty() || width <= 0 || height <= 0) {
                std::cerr << "[ImageWriteJob] Invalid data or path." << std::endl;
                return;
            }

            // 원본 파일명 뒤에 "_out" 접미사를 추가하여 출력 경로 생성 (예: image.jpg -> image_out.png)
            std::filesystem::path output_path = target_path;
            output_path.replace_filename(target_path.stem().string() + "_out.png");

            // 1행당 바이트 수 (stride)
            int stride_in_bytes = width * channels;

            // stb_image_write를 이용한 PNG 파일 저장
            int result = stbi_write_png(
                output_path.string().c_str(),
                width,
                height,
                channels,
                pixel_data.data(),
                stride_in_bytes
            );

            if (result == 0) {
                std::cerr << "[ImageWriteJob] Failed to write image: " << output_path << std::endl;
            } else {
                std::cout << "[ImageWriteJob] Successfully saved: " << output_path << std::endl;
            }
        }
    };

    struct ImageProcessingJob {
        std::vector<uint8_t> pixel_data;
        int width{0};
        int height{0};
        int channels{0};
        std::filesystem::path target_path; // 경로 유지를 위해 전달받는 멤버 변수 추가

        ImageProcessingJob(std::vector<uint8_t>&& data, int w, int h, int ch, std::filesystem::path path)
            : pixel_data(std::move(data)), width(w), height(h), channels(ch), target_path(std::move(path)) {}

        ImageWriteJob action() && {
            std::printf("[image process job]\n");
            if (pixel_data.empty() || width < 5 || height < 5) {
                return ImageWriteJob{ std::move(pixel_data), width, height, channels, std::move(target_path) };
            }

            std::vector<uint8_t> output_data(pixel_data.size(), 0);

            constexpr int K_SIZE = 5;
            constexpr int HALF_K = K_SIZE / 2;
            constexpr int kernel[5][5] = {
                { -1, -1, -1, -1, -1 },
                { -1, -1, -1, -1, -1 },
                { -1, -1, 24, -1, -1 },
                { -1, -1, -1, -1, -1 },
                { -1, -1, -1, -1, -1 }
            };

            for (int y = HALF_K; y < height - HALF_K; ++y) {
                for (int x = HALF_K; x < width - HALF_K; ++x) {
                    for (int c = 0; c < channels; ++c) {

                        if (c == 3 && channels == 4) {
                            int idx = (y * width + x) * channels + c;
                            output_data[idx] = pixel_data[idx];
                            continue;
                        }

                        int sum = 0;

                        for (int ky = -HALF_K; ky <= HALF_K; ++ky) {
                            for (int kx = -HALF_K; kx <= HALF_K; ++kx) {
                                int px = x + kx;
                                int py = y + ky;
                                int pixel_idx = (py * width + px) * channels + c;

                                sum += pixel_data[pixel_idx] * kernel[ky + HALF_K][kx + HALF_K];
                            }
                        }

                        int current_idx = (y * width + x) * channels + c;
                        output_data[current_idx] = static_cast<uint8_t>(std::clamp(sum, 0, 255));
                    }
                }
            }

            //std::printf("[image process job done]\n");
            return ImageWriteJob{ std::move(output_data), width, height, channels, std::move(target_path) };
        }
    };

    struct ImageLoadJob {
        explicit ImageLoadJob(std::filesystem::path target_path, int desired_channels = 3)
            : target(std::move(target_path)), req_channels(desired_channels) {}

        ImageProcessingJob action() && {
            int width = 0;
            int height = 0;
            int channels = 0;

            //std::printf("[image load job]\n");
            unsigned char* raw_pixels = stbi_load(
                target.string().c_str(),
                &width,
                &height,
                &channels,
                req_channels
            );
            //std::printf("[image load job done]\n");
            std::fflush(stdout);
            std::vector<uint8_t> buffer;

            if (raw_pixels) {
                int actual_channels = (req_channels != 0) ? req_channels : channels;
                size_t image_size = static_cast<size_t>(width * height * actual_channels);

                buffer.assign(raw_pixels, raw_pixels + image_size);
                stbi_image_free(raw_pixels);
            } else {
                std::cerr << "[ImageLoadJob] Failed to load image: "
                          << target << " Reason: " << stbi_failure_reason() << std::endl;
            }

            return ImageProcessingJob{
                std::move(buffer),
                width,
                height,
                (req_channels != 0) ? req_channels : channels,
                std::move(target) // 파일 경로를 다음 Job으로 전파
            };
        }

        std::filesystem::path target;
        int req_channels{0};
    };

} // namespace async_mpmc