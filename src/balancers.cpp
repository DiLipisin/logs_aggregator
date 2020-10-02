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
    try {
        while (next_file_number <= files_number) {
            const unsigned file_number = GetNextFileNumber();
            const auto &infile_name = log_dir_path + "/file" + std::to_string(file_number) + ".log";
            LogfileSplitter logfile_splitter(infile_name, tmp_dir_path);
            logfile_splitter.Run();
            std::cout << "logfile=" << infile_name << " splitted to tmp_dir_path=" << tmp_dir_path << std::endl;
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

unsigned InputFilesBalancer::GetNextFileNumber() {
    std::lock_guard<std::mutex> lg(mx);
    const unsigned file_number = next_file_number;
    next_file_number++;
    return file_number;
}

TmpOutputFilesBalancer::TmpOutputFilesBalancer(const SameDateFiles& same_date_files) {
    files = same_date_files;
    next_files_bunch = same_date_files.begin();
}

void TmpOutputFilesBalancer::Run(const std::string& output_dir) {
    try {
        while (next_files_bunch != files.end()) {
            const auto& [file_name, similar_file_paths] = GetNextFilesBunch();
            const auto &facts = PrepareFacts(similar_file_paths);

            std::string outfile_name = output_dir + "/" + file_name;
            std::ofstream outfile(outfile_name, std::ios::app);
            if (!outfile.is_open()) {
                std::cerr << "Temporary facts outfile " << outfile_name
                    << " opening error: " << HandleErrno(errno) << std::endl;
                throw std::runtime_error("OFile open error: " + outfile_name);
            }

            WriteFactsToFile(facts, outfile);
            std::cout << "File bunch " << file_name << " aggregated" << std::endl;
        }
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::pair<std::string, std::unordered_set<std::string>> TmpOutputFilesBalancer::GetNextFilesBunch() {
    std::lock_guard<std::mutex> lg(mx);
    const auto& next_bunch = *next_files_bunch;
    next_files_bunch++;
    return next_bunch;
}
