#include "balansers.h"

#include <iostream>
#include <fstream>
#include <thread>


InputFilesBalancer::InputFilesBalancer(const uint8_t input_files_number) {
    files_number = input_files_number;
    next_file_number = 1;
}

void InputFilesBalancer::Run(const std::string& log_dir_path, const std::string& tmp_dir_path) {
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

TmpOutputFilesBalancer::TmpOutputFilesBalancer(const SimilarFiles& similar_files) {
    files = similar_files;
    next_files_bunch = similar_files.begin();
}

void TmpOutputFilesBalancer::Run(const std::string& output_dir) {
    while (next_files_bunch != files.end()) {
        mx.lock();
        const auto& file_name = next_files_bunch->first;
        const auto& similar_file_paths = next_files_bunch->second;
        next_files_bunch++;
        mx.unlock();


        std::unordered_map<std::string, uint32_t> props;

        for (const auto& file_path: similar_file_paths) {
            std::ifstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "IFile open error: " << file_path;
            }

            std::string tuple;
            while (std::getline(file, tuple)) {
                if (props.find(tuple) == props.end()) {
                    props[tuple] = 1;
                } else {
                    props[tuple] += 1;
                }
            }

            for (const auto& [prop, c]: props) {
                std::cout << "%% " << prop << ": " << c << std::endl;
            }
        }

        std::ofstream outfile(output_dir + "/" + file_name, std::ios::app);
        if (!outfile.is_open()) {
            std::cerr << "OFile open error: " << file_name;
        }
        for (const auto& [tuple, counter]: props) {
            outfile << "{\"props\": [" << tuple << "], \"count\": " << std::to_string(counter) << "}" << std::endl;
        }
    }
}
