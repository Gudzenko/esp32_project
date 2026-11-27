#pragma once

#include <cstddef>
#include <cstdint>

namespace config {

constexpr const char* kBoardName = "esp32";
constexpr const char* kFirmwareVersion = "0.1.0";
constexpr int kProtocolVersion = 1;

constexpr uint32_t kSerialBaud = 115200;
constexpr size_t kMaxLogLineLength = 256;

constexpr const char* kLedName = "led";
constexpr uint8_t kLedPin = 2;
constexpr const char* kBootButtonName = "boot";
constexpr uint8_t kBootButtonPin = 0;
constexpr uint32_t kPwmFrequencyHz = 5000;
constexpr uint8_t kPwmResolutionBits = 12;
constexpr uint32_t kLedBreathPeriodMs = 2000;
constexpr uint32_t kLedBlinkHalfPeriodMs = 500;

constexpr uint32_t kWifiRetryIntervalMs = 5000;

constexpr uint16_t kHttpPort = 80;

constexpr uint16_t kWsPort = 81;
constexpr uint32_t kWsPingIntervalMs = 2500;
constexpr uint32_t kWsPongTimeoutMs = 2000;
constexpr uint8_t kWsMissedPongsToDisconnect = 2;
static_assert(kWsPongTimeoutMs < kWsPingIntervalMs,
              "WebSockets library never drops a lost client if pong timeout >= ping interval");

}  // namespace config
