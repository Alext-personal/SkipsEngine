#pragma once
#include <utility>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <filesystem>
#include <fstream>
#include <chrono>
#include "Core/Log.h"
namespace Gaze {
    inline std::string DumpFileToString(const std::filesystem::path& filepath) {
        std::stringstream ss;
        std::ifstream file(filepath);
        if (!file)
            LOG_ERROR("Failed to open file: ${} ", filepath);
        ss << file.rdbuf();
        return ss.str();
    }
    inline auto GetTime() {
        return std::chrono::high_resolution_clock::now();
    }
    inline std::filesystem::path GetCurrentPath() {
       #ifdef _WIN32
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(nullptr, path, MAX_PATH);
        return std::filesystem::path(path).parent_path();
       #endif
        return "";
    }
}