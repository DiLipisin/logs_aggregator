#pragma once

#include <string>

void RemoveDirectory(const std::string& dir_path);
void MakeDirectory(const std::string& dir_path);

std::string HandleErrno(int errno_value);
