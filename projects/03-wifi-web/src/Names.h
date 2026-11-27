#pragma once

#include <cstring>

inline bool sameName(const char* name, const char* expected) {
    return std::strcmp(name, expected) == 0;
}
