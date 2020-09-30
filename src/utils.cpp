#include "utils.h"

#include <cstring>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void MakeDirectory(const std::string& dir_path) {
    if (fs::exists(dir_path)) {
        fs::remove_all(dir_path);
    }
    fs::create_directory(dir_path);
}

std::string HandleErrno(const int errno_value) {
    size_t size = 1024;
    char* buf = new char[size];
    char* code = strerror_r(errno_value, buf, size);
    const auto& result = "code=" + std::string(code) + ", buf=" + std::string(buf);
    delete [] buf;

    return result;
}