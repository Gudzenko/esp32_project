#include "Log.h"

#include <Arduino.h>
#include <esp_timer.h>

#include <cstdarg>

#include "config.h"

namespace {

constexpr uint64_t kMicrosPerMilli = 1000;

void logLine(const char* level, const char* layer, const char* format, va_list args) {
    char text[config::kMaxLogLineLength];
    vsnprintf(text, sizeof(text), format, args);
    const uint64_t uptimeMs = esp_timer_get_time() / kMicrosPerMilli;
    Serial.printf("%llu [%s] %s %s\n", uptimeMs, layer, level, text);
}

}  // namespace

void logInfo(const char* layer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logLine("INFO", layer, format, args);
    va_end(args);
}

void logWarn(const char* layer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logLine("WARN", layer, format, args);
    va_end(args);
}

void logError(const char* layer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logLine("ERROR", layer, format, args);
    va_end(args);
}
