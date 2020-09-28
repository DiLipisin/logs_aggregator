#pragma once

#include <experimental/filesystem>

#include "input_log_file_handler.h"

namespace {
    namespace fs = std::experimental::filesystem;
}

class FileBalancer {
public:
    explicit FileBalancer(const uint8_t input_files_number) {
        files_number = input_files_number;
        next_file_number = 1;
    }
    ~FileBalancer() = default;

    void Run(const std::string& log_dir_path, const std::string& tmp_dir_path) {
        while (next_file_number <= files_number) {
            mx.lock();
            const unsigned file_number = next_file_number;
            next_file_number++;
            mx.unlock();

            const auto &infile_name = log_dir_path + "/file" + std::to_string(file_number) + ".log";
            std::cout << std::this_thread::get_id() <<  " * infile_name: " << infile_name << std::endl;
            std::cout << std::this_thread::get_id() <<  " ** tmp_dir_path: " << tmp_dir_path << std::endl;

            InputLogFileHandler(infile_name, tmp_dir_path);
        }
    }
private:
    uint8_t files_number;
    uint8_t next_file_number;
    std::mutex mx;
};