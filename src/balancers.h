#pragma once

#include <experimental/filesystem>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

using SameDateFiles = std::unordered_map<std::string, std::unordered_set<std::string>>;

class InputFilesBalancer {
public:
    explicit InputFilesBalancer(uint8_t input_files_number);
    void Run(const std::string& log_dir_path, std::string tmp_dir_path);

private:
    uint8_t files_number;
    uint8_t next_file_number;
    std::mutex mx;
};

class TmpOutputFilesBalancer {
public:
    explicit TmpOutputFilesBalancer(const SameDateFiles& same_date_files);
    void Run(const std::string& output_dir);

private:
    SameDateFiles files;
    SameDateFiles::const_iterator next_files_bunch;
    std::mutex mx;
};