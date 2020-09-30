#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <experimental/filesystem>

class LogfileSplitter {
public:
    LogfileSplitter(const std::string& infile_name, const std::string& abs_dir_path);
    ~LogfileSplitter();

private:
    std::ofstream* FindOutfile(const std::string& file_name);

    std::string dir_path;
    std::ifstream infile;
    std::unordered_map<std::string, std::ofstream*> outfiles;
};
