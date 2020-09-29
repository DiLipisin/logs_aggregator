#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <vector>

#include <experimental/filesystem>

#include "helpers.h"
#include "input_log_file_handler.h"
#include "balansers.h"

namespace {
    namespace fs = std::experimental::filesystem;

    fs::path MakeDirectory(const unsigned number) {
        static const fs::path prev_dir = fs::temp_directory_path() / "dir";
        fs::create_directory(prev_dir);

        fs::path dir_path = prev_dir / std::to_string(number);
        if (fs::exists(dir_path)) {
            fs::remove_all(dir_path);
        }
        fs::create_directory(dir_path);

        return dir_path;
    }

    std::vector<fs::path> HandleInputFiles(const std::string& input_dir_path, const uint8_t files_number,
                                           const uint8_t threads_number) {
        std::vector<std::thread> threads;
        threads.reserve(threads_number);
        std::vector<fs::path> tmp_output_dir_paths;
        tmp_output_dir_paths.reserve(threads_number);

        InputFilesBalancer balancer(files_number);
        for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
            const auto& output_dir_path = MakeDirectory(thread_num);
            std::thread th(
                [&balancer, &input_dir_path, &output_dir_path]() {
                    balancer.Run(input_dir_path, output_dir_path);
                }
            );
            threads.push_back(std::move(th));
            tmp_output_dir_paths.push_back(output_dir_path);
        }
        for (auto& th: threads) {
            th.join();
        }

        return tmp_output_dir_paths;
    }

    std::unordered_map<std::string, std::unordered_set<std::string>> AggregateDirFiles(
            const std::vector<fs::path>& tmp_dir_paths) {
        std::unordered_map<std::string, std::unordered_set<std::string>> similar_files;

        for (const auto& tmp_dir_path: tmp_dir_paths) {
            for (auto& file: fs::directory_iterator(tmp_dir_path)) {
                std::cout << "tmp_out_file: " << file.path() << std::endl;
                if (similar_files.find(file.path().filename()) == similar_files.end()) {
                    similar_files[file.path().filename()] = { file.path().string() };
                } else {
                    similar_files[file.path().filename()].insert(file.path().string());
                }
            }
        }

        return similar_files;
    }

    void HandleTmpOutputFiles(const uint8_t threads_number, const SimilarFiles& similar_files,
                              const std::string& tmp_output_dir) {
        std::vector<std::thread> threads;
        threads.reserve(threads_number);

        TmpOutputFilesBalancer balancer(similar_files);
        for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
            if (fs::exists(tmp_output_dir)) {
                fs::remove_all(tmp_output_dir);
            }
            fs::create_directory(tmp_output_dir);
            std::thread th([&balancer, &tmp_output_dir]() { balancer.Run(tmp_output_dir); });
            threads.push_back(std::move(th));
        }

        for (auto& th: threads) {
            th.join();
        }
    }

    std::string PreparePropsList(const std::string& file_name) {
        std::ifstream file(file_name);
        if (!file.is_open()) {
            std::cerr << "IFile open error: " << file_name << std::endl;
        }

        std::string props_list;
        std::string props;
        while (std::getline(file, props)) {
            props_list += props + ", ";
        }
        props_list.pop_back();
        props_list.pop_back();

        return props_list;
    }

    void PrepareResultFile(const std::string& tmp_dir, const std::string& target_dir) {
        if (fs::exists(target_dir)) {
            fs::remove_all(target_dir);
        }
        fs::create_directory(target_dir);
        std::ofstream target_file(target_dir + "/agr.txt");
        if (!target_file.is_open()) {
            std::cerr << "OFile open error: agr.txt" << std::endl;
        }
        target_file << "{";
        std::string prev_date;
        std::string prev_fact_name;
        for(const auto& tmp_file: fs::directory_iterator(tmp_dir)) {
            const auto& file_name = tmp_file.path().filename().string();
            const auto& cur_date = file_name.substr(0, 10);
            const auto& cur_fact_name = file_name.substr((11));
            std::cout << "##### " << file_name << " " << cur_date << " " << cur_fact_name << std::endl;
            if (cur_date != prev_date) {
                target_file << "\"" << cur_date << "\": {"
                    << "\"" << cur_fact_name << "\": ["
                    << PreparePropsList(tmp_file.path().string())
                    << "]";
            } else {
                target_file << ", \"" << cur_fact_name << "\": ["
                    << PreparePropsList(tmp_file.path().string())
                    << "]";
            }

        }
    }
}

// 0 - Ok
// 1 - Invalid argument
// 2 -
int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Invalid number of arguments. You need enter binary file "
                     "name and threads number" << std::endl;
        return 1;
    }

    const std::string input_dir_path = argv[1];

    char* er;
    const int files_number = strtol(argv[2], &er, 10);
    if (files_number < 1) {
        std::cerr << "Invalid number of input files: " << argv[2] << std::endl;
        return 1;
    }

    const int threads_number = strtol(argv[3], &er, 10);
    if (threads_number < 1) {
        std::cerr << "Invalid number of threads: " << argv[3] << std::endl;
        return 1;
    }

    const auto& tmp_output_dir_paths = HandleInputFiles(input_dir_path, files_number, threads_number);
    const auto& similar_files = AggregateDirFiles(tmp_output_dir_paths);

    const auto& tmp_output_dir = "/tmp/dir2";
    HandleTmpOutputFiles(threads_number, similar_files, tmp_output_dir);
    PrepareResultFile(tmp_output_dir, "/tmp/res");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
