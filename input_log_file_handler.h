#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <experimental/filesystem>

class InputLogFileHandler {
public:
    explicit InputLogFileHandler(const std::string& infile_name,
                                 const std::experimental::filesystem::path& abs_dir_path);
    InputLogFileHandler(const InputLogFileHandler& other) = delete;
    InputLogFileHandler(InputLogFileHandler&& other) = delete;
    InputLogFileHandler& operator=(const InputLogFileHandler& other) = delete;
    InputLogFileHandler& operator=(InputLogFileHandler&& other) = delete;
    ~InputLogFileHandler();

private:
    std::ofstream* FindOutfile(const std::string& file_name);

    std::experimental::filesystem::path dir_path;
    std::ifstream infile;
    std::unordered_map<std::string, std::ofstream*> outfiles;
};
