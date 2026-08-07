#pragma once
#include <utility>
#include <filesystem>
#include <fstream>
#define TO_EVENT_FN(fn) \
    [this](auto&&... args) -> decltype(auto)\
    { \
        return this->fn(std::forward<decltype(args)>(args)...); \
    }
static std::string DumpFileToString(const std::filesystem::path& filepath) {
    std::stringstream ss;
    std::ifstream file(filepath);
    ss << file.rdbuf();
    return ss.str();
}