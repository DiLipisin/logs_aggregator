#pragma once

#include <string>

void MakeDirectory(const std::string& dir_path);

std::string HandleErrno(int errno_value);
