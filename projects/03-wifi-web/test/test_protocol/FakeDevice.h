#pragma once

#include <optional>
#include <string>

#include "Device.h"
#include "Names.h"

class FakeLamp : public Device {
public:
    static constexpr int kMaxLevel = 10;

    explicit FakeLamp(const char* name) : name_(name) {}

    const char* name() const override {
        return name_;
    }

    const char* type() const override {
        return "lamp";
    }

    void describe(JsonObject props) const override {
        props["level"]["min"] = 0;
        props["level"]["max"] = kMaxLevel;
        props["temperature"]["readonly"] = true;
    }

    void get(JsonObject data) const override {
        data["level"] = level_;
        data["temperature"] = kTemperature;
    }

    std::optional<ProtocolError> set(JsonObjectConst values, JsonObject data) override {
        std::optional<int> newLevel;
        for (JsonPairConst value : values) {
            const char* prop = value.key().c_str();
            if (sameName(prop, "temperature")) {
                return readOnly("temperature");
            }
            if (!sameName(prop, "level")) {
                return unknownProp(prop);
            }
            if (!value.value().is<int>() || value.value().as<int>() < 0 ||
                value.value().as<int>() > kMaxLevel) {
                return badValue("level", "allowed: 0..10");
            }
            newLevel = value.value().as<int>();
        }
        if (newLevel) {
            level_ = *newLevel;
            notifyChanged();
        }
        get(data);
        return std::nullopt;
    }

    void changeOnBoard(int level) {
        level_ = level;
        notifyChanged();
    }

    int level() const {
        return level_;
    }

private:
    static constexpr int kTemperature = 21;

    const char* name_;
    int level_ = 0;
};
