#pragma once

#include <stdexcept>

class IFileOpenException: public std::runtime_error {
public:
    explicit IFileOpenException(const std::string& msg): std::runtime_error(msg) {}
};

class OFileOpenException: public std::runtime_error {
public:
    explicit OFileOpenException(const std::string& msg): std::runtime_error(msg) {}
};

class JsonParseException: public std::runtime_error {
public:
    explicit JsonParseException(const std::string& msg): std::runtime_error(msg) {}
};