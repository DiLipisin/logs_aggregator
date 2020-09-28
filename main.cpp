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

namespace {
    namespace fs = std::experimental::filesystem;
    const fs::path tmp_dir_pref = fs::temp_directory_path() / "dir";

    fs::path MakeDirectory(const unsigned thread_number) {
        fs::create_directory(tmp_dir_pref);
        fs::path dir_path = tmp_dir_pref / std::to_string(thread_number);
        if (fs::exists(dir_path)) {
            fs::remove_all(dir_path);
        }
        fs::create_directory(dir_path);

        return dir_path;
    }

    std::vector<fs::path> HandleInputFiles(const std::string& log_dir_path, const uint8_t files_number,
                                           const uint8_t threads_number) {
        std::vector<std::thread> threads;
        threads.reserve(threads_number);
        std::vector<fs::path> tmp_dir_paths;
        tmp_dir_paths.reserve(threads_number);

        FileBalancer balancer(files_number);
        for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
            const auto& tmp_dir_path = MakeDirectory(thread_num);
            std::thread th([&balancer, &log_dir_path, &tmp_dir_path]() { balancer.Run(log_dir_path, tmp_dir_path); });
            threads.push_back(std::move(th));
            tmp_dir_paths.push_back(tmp_dir_path);
        }

        for (auto& th: threads) {
            th.join();
        }

        return tmp_dir_paths;
    }

    std::unordered_map<std::string, std::unordered_set<std::string>> AggregateDirFiles(
            const std::vector<fs::path>& tmp_dir_paths) {
        std::unordered_map<std::string, std::unordered_set<std::string>> dir_files;

        for (const auto& tmp_dir_path: tmp_dir_paths) {
            std::unordered_set<std::string> files;
            std::cout << "$ " << tmp_dir_path.string() << std::endl;
            for (auto& file: fs::directory_iterator(tmp_dir_path)) {
                files.insert(file.path().filename());
                std::cout << "$$ " << file.path().filename() << std::endl;
            }
            dir_files[tmp_dir_path.string()] = files;
        }

        return dir_files;
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Invalid number of arguments. You need enter binary file "
                     "name and threads number" << std::endl;
        return 1;
    }

    const std::string log_dir_path = argv[1];

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

    const auto& tmp_dir_paths = HandleInputFiles(log_dir_path, files_number, threads_number);
    const int tmp_dirs_number = tmp_dir_paths.size();
    if (tmp_dirs_number != files_number) {
        std::cerr << "Temporary directories number is not equal input files number " << tmp_dirs_number << " " << files_number << std::endl;
        return 2;
    }

    if (tmp_dirs_number) {
        std::unordered_map<std::string, std::unordered_set<std::string>> dir_files;
//        TODO: calculate the largest dir
        if (tmp_dirs_number > 1) {
            dir_files = AggregateDirFiles(tmp_dir_paths);
        }

        for (int thread_num = 0; thread_num < threads_number; thread_num++) {
            std::thread th;
        }

        for (const auto& dir_entry: fs::directory_iterator(tmp_dir_paths[0])) {
            const auto& merge_file_path = dir_entry.path();
            std::fstream merge_file(merge_file_path, std::ios::out|std::ios::in);
            if (!merge_file.is_open()) {
                std::cerr << "IOFile open error: " << merge_file_path;
                return 3;
            }

            std::unordered_map<std::string, uint32_t> props;
            std::string tuple;
            while (std::getline(merge_file, tuple)) {
                if (props.find(tuple) == props.end()) {
                    props[tuple] = 1;
                } else {
                    props[tuple] += 1;
                }
            }

            for (const auto& [prop, c]: props) {
                std::cout << "%% " << prop << std::endl;
            }

            if (!dir_files.empty()) {
                for (auto& [dir, files]: dir_files) {
                    const auto& same_name_file = files.find(merge_file_path.filename());
                    if (same_name_file == files.end()) {
                        continue;
                    }

                    std::ifstream file(dir + "/" + *same_name_file);
                    if (!merge_file.is_open()) {
                        std::cerr << "IFile open error: " << merge_file_path;
                        return 3;
                    }

                    std::string tuple;
                    while (merge_file >> tuple) {
                        if (props.find(tuple) == props.end()) {
                            props[tuple] = 1;
                        } else {
                            props[tuple] += 1;
                        }
                    }
                }
            }
        }
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
