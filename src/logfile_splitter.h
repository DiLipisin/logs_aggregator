#pragma once

#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_map>

class LogfileSplitter {
public:
    LogfileSplitter(const std::string& infile_name, const std::string& abs_dir_path);

private:
    std::ofstream& FindOutfile(const std::string& file_name);

    std::string dir_path;
    std::ifstream infile;
    std::unordered_map<std::string, std::ofstream> outfiles;
};
