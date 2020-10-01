#include <chrono>
#include <iostream>
#include <string>

#include "src/balancers.h"
#include "src/handlers.h"
#include "src/utils.h"

// 0 - Ok
// 1 - Invalid argument
int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Invalid number of arguments. Input 4 arguments: "
                     "log files directory, files number, threads number, result output directory" << std::endl;
        return 1;
    }

    const std::string input_dir_path = argv[1];

    const int files_number = (int)strtol(argv[2], nullptr, 10);
    if (files_number < 1) {
        std::cerr << "Invalid number of input files: " << argv[2] << std::endl;
        return 1;
    }

    const int threads_number = (int)strtol(argv[3], nullptr, 10);
    if (threads_number < 1) {
        std::cerr << "Invalid number of threads: " << argv[3] << std::endl;
        return 1;
    }

    const std::string output_dir_path = argv[4];

    namespace fs = std::experimental::filesystem;
    static const std::string splitted_files_dir = (fs::temp_directory_path() / "dir").string();
    MakeDirectory(splitted_files_dir);
    auto timer = std::chrono::system_clock::now();
    const auto& tmp_output_dir_paths = SplitLogsByDate(
            input_dir_path, files_number, threads_number, splitted_files_dir);
    std::cout << "SplitLogsByDate: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count()
        << std::endl;

    timer = std::chrono::system_clock::now();
    const auto& aggregated_file_names = GetSameNameFiles(tmp_output_dir_paths);
    std::cout << "GetSameNameFiles: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count()
        << std::endl;

    static const std::string aggregated_files_dir = (fs::temp_directory_path() / "dir2").string();
    MakeDirectory(aggregated_files_dir);
    timer = std::chrono::system_clock::now();
    AggregateSameDateFiles(threads_number, aggregated_file_names, aggregated_files_dir);
    std::cout << "AggregateSameDateFiles: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count()
        << std::endl;

    MakeDirectory(output_dir_path);
    timer = std::chrono::system_clock::now();
    PrepareResultFile(aggregated_files_dir, output_dir_path);
    std::cout << "PrepareResultFile: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count()
        << std::endl;

    return 0;
}
