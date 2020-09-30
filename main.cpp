#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <vector>

#include "balansers.h"
#include "helpers.h"

// 0 - Ok
// 1 - Invalid argument
int main(int argc, char** argv) {
    if (argc != 5) {
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

    const std::string output_dir_path = argv[4];

    namespace fs = std::experimental::filesystem;
    static const std::string splitted_files_dir = (fs::temp_directory_path() / "dir").string();
    MakeDirectory(splitted_files_dir);
    const auto& tmp_output_dir_paths = SplitLogsByDateAndFactName(
            input_dir_path, files_number, threads_number, splitted_files_dir);

    const auto& aggregated_file_names = GetSameNameFiles(tmp_output_dir_paths);

    static const std::string aggregated_files_dir = (fs::temp_directory_path() / "dir2").string();
    MakeDirectory(aggregated_files_dir);
    AggregateSameDateAndFactNameFiles(threads_number, aggregated_file_names, aggregated_files_dir);

    MakeDirectory(output_dir_path);
    PrepareResultFile(aggregated_files_dir, output_dir_path);

    std::cout << "Hello, World1!" << std::endl;
    return 0;
}
