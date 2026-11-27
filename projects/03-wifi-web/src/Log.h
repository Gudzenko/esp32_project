#pragma once

void logInfo(const char* layer, const char* format, ...) __attribute__((format(printf, 2, 3)));
void logWarn(const char* layer, const char* format, ...) __attribute__((format(printf, 2, 3)));
void logError(const char* layer, const char* format, ...) __attribute__((format(printf, 2, 3)));
