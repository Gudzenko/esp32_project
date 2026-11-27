#pragma once

#include <cstddef>
#include <cstdint>

namespace config {

constexpr const char* kBoardName = "esp32";
constexpr const char* kFirmwareVersion = "0.1.0";

constexpr uint32_t kSerialBaud = 115200;
constexpr size_t kMaxLogLineLength = 256;

constexpr uint8_t kLedPin = 2;
constexpr uint32_t kLedBlinkIntervalMs = 250;

constexpr uint32_t kWifiRetryIntervalMs = 5000;

constexpr uint16_t kHttpPort = 80;

}  // namespace config
