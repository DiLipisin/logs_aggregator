#pragma once

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <vector>

#include <experimental/filesystem>

void MakeDirectory(const std::string& dir_path);

std::vector<std::experimental::filesystem::path> SplitLogsByDateAndFactName(
        const std::string& input_dir_path, const uint8_t files_number,
        const uint8_t threads_number, const std::string& output_dir_pat);

std::unordered_map<std::string, std::unordered_set<std::string>> GetSameNameFiles(
        const std::vector<std::experimental::filesystem::path>& tmp_dir_paths);

void AggregateSameDateAndFactNameFiles(const uint8_t threads_number,
                          const std::unordered_map<std::string, std::unordered_set<std::string>>& similar_files,
                          const std::string& tmp_output_dir);

void PrepareResultFile(const std::string& tmp_dir, const std::string& target_dir);
