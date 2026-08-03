//
// Created by lenovo on 2026-08-04.
//

#ifndef MCMS_FILELOAD_HPP
#define MCMS_FILELOAD_HPP

#include <vector>
#include <cinttypes>
#include <filesystem>
#include <iostream>

struct FileLoadJob {

    FileLoadJob(std::filesystem::path target_path) : target(std::move(target_path)) {

    }

    void action() {

    }

    std::filesystem::path target;
    std::vector<uint8_t> data;

};

class FileLoad {
public:
    FileLoad(std::filesystem::path target_dir) : m_target(std::move(target_dir)) {
        if (!std::filesystem::is_directory(m_target)) {
            return;
        }
        make_loading_jobs();
    }


private:
    void make_loading_jobs() {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_target)) {

       }
    }


    std::filesystem::path m_target;
};

#endif //MCMS_FILELOAD_HPP
