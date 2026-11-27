#pragma once

#include <esp_timer.h>

#include <cstdint>

inline uint64_t uptimeMs() {
    constexpr uint64_t kMicrosPerMilli = 1000;
    return esp_timer_get_time() / kMicrosPerMilli;
}
