#include "helpers.h"
#include "balansers.h"

#include <sstream>
#include <set>

namespace fs = std::experimental::filesystem;

void MakeDirectory(const std::string& dir_path) {
    if (fs::exists(dir_path)) {
        fs::remove_all(dir_path);
    }
    fs::create_directory(dir_path);
}

std::vector<fs::path> SplitLogsByDateAndFactName(const std::string& input_dir_path, const uint8_t files_number,
                                                 const uint8_t threads_number, const std::string& output_dir_path) {
    std::vector<std::thread> threads;
    threads.reserve(threads_number);
    std::vector<fs::path> tmp_output_dir_paths;
    tmp_output_dir_paths.reserve(threads_number);

    InputFilesBalancer balancer(files_number);
    for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
        const fs::path& tmp_output_dir_path = fs::path(output_dir_path) / std::to_string(thread_num);
        MakeDirectory(tmp_output_dir_path);
        std::cout << "thread=`" << thread_num << "` writes to dir=`" << tmp_output_dir_path << "`" << std::endl;

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
//            std::cout << "tmp_out_file: " << file.path() << std::endl;
            if (aggregated_files.find(file.path().filename()) == aggregated_files.end()) {
                aggregated_files[file.path().filename()] = { file.path().string() };
            } else {
                aggregated_files[file.path().filename()].insert(file.path().string());
            }
        }
    }

    return aggregated_files;
}

void AggregateSameDateAndFactNameFiles(const uint8_t threads_number, const SimilarFiles& similar_files,
                                       const std::string& tmp_output_dir) {
    std::vector<std::thread> threads;
    threads.reserve(threads_number);

    TmpOutputFilesBalancer balancer(similar_files);
    for (int thread_num = 1; thread_num <= threads_number; thread_num++) {
        std::thread th([&balancer, tmp_output_dir]() { balancer.Run(tmp_output_dir); });
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

//    std::stringstream target_file;

    struct FilePathsComparator {
        bool operator()(const fs::path& lhs, const fs::path& rhs) const {
            return lhs.filename().string() < rhs.filename().string();
        }
    };
    std::set<fs::path, FilePathsComparator> file_paths;
    for (const auto& tmp_file: fs::directory_iterator(tmp_dir)) {
        file_paths.insert(tmp_file.path());
    }

    target_file << "{";
    bool date_open = false;
    bool start = true;
    std::string prev_date;
    std::string prev_fact_name;
    for (const auto& file_path: file_paths) {
        const auto& file_name = file_path.filename().string();
        const auto& cur_date = file_name.substr(0, 10);
        const auto& cur_fact_name = file_name.substr((11));
        if (date_open && cur_date != prev_date) {
            target_file << "}";
//            std::cout << "3 " << target_file.str()<< std::endl;
            date_open = false;
        }
        if (!start) {
            target_file << ", ";
//            std::cout << "4 " << target_file.str() << std::endl;
        }
//        std::cout << "$$$ " << cur_date << " " << prev_date << std::endl;
        if (cur_date != prev_date) {
            target_file << "\"" << cur_date << "\": {"
                        << "\"" << cur_fact_name << "\": ["
                        << PreparePropsList(file_path.string())
                        << "]";
//            std::cout << "1 " << target_file.str() << std::endl;
            date_open = true;
        } else {
            target_file << "\"" << cur_fact_name << "\": ["
                        << PreparePropsList(file_path.string())
                        << "]";
//            std::cout << "2 " << target_file.str()<< std::endl;
        }
        prev_date = cur_date;
        start = false;
    }
    target_file << "}}";
}