#pragma once

// Copy this file to secrets.h and fill in. secrets.h is git-ignored: never commit real passwords.

#include <IPAddress.h>

namespace secrets {

constexpr const char* kWifiSsid = "your-network-2.4GHz";
constexpr const char* kWifiPassword = "your-password";

const IPAddress kStaticIp(192, 168, 1, 200);
const IPAddress kGateway(192, 168, 1, 1);
const IPAddress kSubnet(255, 255, 255, 0);

}  // namespace secrets
