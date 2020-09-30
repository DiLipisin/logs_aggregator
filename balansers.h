#pragma once

#include <experimental/filesystem>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "input_log_file_handler.h"

using SimilarFiles = std::unordered_map<std::string, std::unordered_set<std::string>>;

class InputFilesBalancer {
public:
    explicit InputFilesBalancer(const uint8_t input_files_number);
    InputFilesBalancer(const InputFilesBalancer& other) = delete;
    InputFilesBalancer(InputFilesBalancer&& other) = delete;
    ~InputFilesBalancer() = default;

    void Run(const std::string& log_dir_path, const std::string tmp_dir_path);
private:
    uint8_t files_number;
    uint8_t next_file_number;
    std::mutex mx;
};

class TmpOutputFilesBalancer {
public:
    explicit TmpOutputFilesBalancer(const SimilarFiles& similar_files);
    TmpOutputFilesBalancer(const TmpOutputFilesBalancer& other) = delete;
    TmpOutputFilesBalancer(TmpOutputFilesBalancer&& other) = delete;
    ~TmpOutputFilesBalancer() = default;

    void Run(const std::string& output_dir);
private:
    SimilarFiles files;
    SimilarFiles::const_iterator next_files_bunch;
    std::mutex mx;
};