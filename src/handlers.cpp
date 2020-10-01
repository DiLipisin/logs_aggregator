#include "handlers.h"

#include <iostream>
#include <set>
#include <stdexcept>

#include "balancers.h"
#include "logfile_splitter.h"
#include "utils.h"

namespace {
    namespace fs = std::experimental::filesystem;

    struct FilePathsComparator {
        bool operator()(const fs::path &lhs, const fs::path &rhs) const {
            return lhs.filename().string() < rhs.filename().string();
        }
    };

    void WriteDateFile(const std::string &file_name, std::ostream &target_file) {
        std::ifstream file(file_name);
        if (!file.is_open()) {
            std::cerr << "One of same date files " << file_name
                      << " opening error: " << HandleErrno(errno) << std::endl;
            throw std::runtime_error("IFile open error: " + file_name);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (file.peek() == EOF) {
                target_file << line;
            } else {
                target_file << line + ", ";
            }
        }
    }
}

std::vector<fs::path> SplitLogsByDate(const std::string& input_dir_path, const uint8_t files_number,
                                                 const uint8_t threads_number, const std::string& output_dir_path) {
    std::vector<std::thread> threads;
    threads.reserve(threads_number);
    std::vector<fs::path> tmp_output_dir_paths;
    tmp_output_dir_paths.reserve(threads_number);

    InputFilesBalancer balancer(files_number);
    for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
        const fs::path& tmp_output_dir_path = fs::path(output_dir_path) / std::to_string(thread_num);
        MakeDirectory(tmp_output_dir_path);
        std::cout << "thread=" << thread_num << " writes to dir=" << tmp_output_dir_path << std::endl;

        std::thread th(
                [&balancer, &input_dir_path, tmp_output_dir_path]() {
                    balancer.Run(input_dir_path, tmp_output_dir_path);
                }
        );
        threads.push_back(std::move(th));
        tmp_output_dir_paths.push_back(tmp_output_dir_path);
    }
    for (auto& th: threads) {
        th.join();
    }

    return tmp_output_dir_paths;
}

std::unordered_map<std::string, std::unordered_set<std::string>> GetSameNameFiles(
        const std::vector<fs::path>& tmp_dir_paths) {
    std::unordered_map<std::string, std::unordered_set<std::string>> aggregated_files;

    for (const auto& tmp_dir_path: tmp_dir_paths) {
        for (auto& file: fs::directory_iterator(tmp_dir_path)) {
            if (aggregated_files.find(file.path().filename()) == aggregated_files.end()) {
                aggregated_files[file.path().filename()] = { file.path().string() };
            } else {
                aggregated_files[file.path().filename()].insert(file.path().string());
            }
        }
    }

    return aggregated_files;
}

void AggregateSameDateFiles(const uint8_t threads_number, const SameDateFiles& same_date_files,
                                       const std::string& tmp_output_dir) {
    std::vector<std::thread> threads;
    threads.reserve(threads_number);

    TmpOutputFilesBalancer balancer(same_date_files);
    for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
        std::thread th([&balancer, tmp_output_dir]() { balancer.Run(tmp_output_dir); });
        threads.push_back(std::move(th));
    }

    for (auto& th: threads) {
        th.join();
    }
}

void PrepareResultFile(const std::string& tmp_dir, const std::string& target_dir) {
    if (fs::exists(target_dir)) {
        fs::remove_all(target_dir);
    }
    fs::create_directory(target_dir);
    std::ofstream target_file(target_dir + "/agr.txt");
    if (!target_file.is_open()) {
        throw std::runtime_error("OFile open error: agr.txt");
    }

    std::set<fs::path, FilePathsComparator> file_paths;
    for (const auto& tmp_file: fs::directory_iterator(tmp_dir)) {
        file_paths.insert(tmp_file.path());
    }

    target_file << "{";
    bool date_open = false;
    bool start = true;
    std::string prev_date;
    for (const auto& file_path: file_paths) {
        const auto& file_name = file_path.filename().string();
        const auto& cur_date = file_name;
        if (date_open && cur_date != prev_date) {
            target_file << "}";
            date_open = false;
        }
        if (!start) {
            target_file << ", ";
        }
        if (cur_date != prev_date) {
            target_file << "\"" << cur_date << "\": {";
            WriteDateFile(file_path.string(), target_file);
            date_open = true;
        } else {
            WriteDateFile(file_path.string(), target_file);
        }
        prev_date = cur_date;
        start = false;
    }
    target_file << "}}";
}