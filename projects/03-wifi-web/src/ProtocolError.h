#pragma once

#include <cstdint>
#include <string>

enum class ErrorCode : uint8_t {
    kBadJson,
    kBadRequest,
    kMissingId,
    kUnknownCmd,
    kUnknownDevice,
    kUnknownProp,
    kReadOnly,
    kBadValue,
    kBusy,
};

struct ProtocolError {
    ErrorCode code;
    std::string field;
    std::string message;
};

inline const char* errorCodeName(ErrorCode code) {
    switch (code) {
        case ErrorCode::kBadJson:
            return "bad_json";
        case ErrorCode::kBadRequest:
            return "bad_request";
        case ErrorCode::kMissingId:
            return "missing_id";
        case ErrorCode::kUnknownCmd:
            return "unknown_cmd";
        case ErrorCode::kUnknownDevice:
            return "unknown_device";
        case ErrorCode::kUnknownProp:
            return "unknown_prop";
        case ErrorCode::kReadOnly:
            return "read_only";
        case ErrorCode::kBadValue:
            return "bad_value";
        case ErrorCode::kBusy:
            return "busy";
    }
    return "unknown";
}
