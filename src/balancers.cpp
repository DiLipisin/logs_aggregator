#include "balancers.h"

#include <iostream>
#include <map>
#include <stdexcept>

#include "logfile_splitter.h"
#include "utils.h"

namespace {
    std::map<std::string, std::map<std::string, uint32_t>> PrepareFacts(
            const std::unordered_set<std::string>& similar_file_paths) {
        std::map<std::string, std::map<std::string, uint32_t>> facts;

        for (const auto& file_path: similar_file_paths) {
            std::ifstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "One of same date files " << file_path
                          << " opening error: " << HandleErrno(errno) << std::endl;
                throw std::runtime_error("IFile open error: " + file_path);
            }

            std::string line;
            while (std::getline(file, line)) {
                const auto pipe = line.find('|');
                std::string fact_name = line.substr(0, pipe);
                std::string props = line.substr(pipe + 1);

                if (facts.find(fact_name) == facts.end()) {
                    facts[fact_name] = {{props, 1}};
                } else if (facts[fact_name].find(props) == facts[fact_name].end()) {
                    facts[fact_name].insert({props, 1});
                } else {
                    facts[fact_name][props] += 1;
                }
            }
        }

        return facts;
    }

    void WriteFactsToFile(const std::map<std::string, std::map<std::string, uint32_t>>& facts,
                          std::ofstream& outfile) {
        for (const auto& [fact_name, props]: facts) {
            outfile << "\"" << fact_name << "\": [";
            if (props.size() > 1) {
                for (auto it = props.begin(); it != std::prev(props.end(), 1); it++) {
                    outfile << "{\"props\": [" << it->first << "], "
                            << "\"count\": " << std::to_string(it->second) << "}, ";
                }
            }
            auto it = std::prev(props.end(), 1);
            outfile << "{\"props\": [" << it->first << "], "
                    << "\"count\": " << std::to_string(it->second) << "}]" << std::endl;
        }
    }
}

InputFilesBalancer::InputFilesBalancer(const uint8_t input_files_number) {
    files_number = input_files_number;
    next_file_number = 1;
}

void InputFilesBalancer::Run(const std::string& log_dir_path, const std::string tmp_dir_path) {
    while (next_file_number <= files_number) {
        mx.lock();
        const unsigned file_number = next_file_number;
        next_file_number++;
        mx.unlock();

        const auto &infile_name = log_dir_path + "/file" + std::to_string(file_number) + ".log";
        std::cout << "logfile=" << infile_name << " split to tmp_dir_path=" << tmp_dir_path << std::endl;
        LogfileSplitter(infile_name, tmp_dir_path);
    }
}

TmpOutputFilesBalancer::TmpOutputFilesBalancer(const SameDateFiles& same_date_files) {
    files = same_date_files;
    next_files_bunch = same_date_files.begin();
}

void TmpOutputFilesBalancer::Run(const std::string& output_dir) {
    try {
        while (next_files_bunch != files.end()) {
            mx.lock();
            const auto &file_name = next_files_bunch->first;
            const auto &similar_file_paths = next_files_bunch->second;
            next_files_bunch++;
            mx.unlock();

            const auto &facts = PrepareFacts(similar_file_paths);

            std::string outfile_name = output_dir + "/" + file_name;
            std::ofstream outfile(outfile_name, std::ios::app);
            if (!outfile.is_open()) {
                std::cerr << "Temporary facts outfile " << outfile_name
                    << " opening error: " << HandleErrno(errno) << std::endl;
                throw std::runtime_error("OFile open error: " + outfile_name);
            }

            WriteFactsToFile(facts, outfile);
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}
