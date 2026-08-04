#pragma once
#include <utility>
#define TO_EVENT_FN(fn) \
    [this](auto&&... args) -> decltype(auto)\
    { \
        return this->fn(std::forward<decltype(args)>(args)...); \
    }