#include "Log.h"

#include <Arduino.h>

#include <cstdarg>

#include "Uptime.h"
#include "config.h"

namespace {

void logLine(const char* level, const char* layer, const char* format, va_list args) {
    char text[config::kMaxLogLineLength];
    vsnprintf(text, sizeof(text), format, args);
    Serial.printf("%llu [%s] %s %s\n", uptimeMs(), layer, level, text);
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
