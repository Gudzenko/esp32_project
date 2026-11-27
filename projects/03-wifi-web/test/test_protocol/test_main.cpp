#include <unity.h>

#include <string>
#include <vector>

#include "Devices.h"
#include "FakeDevice.h"
#include "Protocol.h"

namespace {

constexpr uint64_t kTime = 1234;

FakeLamp lamp("lamp");
FakeLamp spare("spare");
Devices devices({&lamp, &spare});
Protocol protocol("test-board", "9.9.9", 1, devices);
std::vector<std::string> events;

JsonDocument parse(const std::string& text) {
    JsonDocument document;
    TEST_ASSERT_FALSE(deserializeJson(document, text));
    return document;
}

JsonDocument handle(const std::string& request) {
    return parse(protocol.handle(request.c_str(), request.size(), kTime));
}

void assertError(const std::string& request, const char* code) {
    const JsonDocument reply = handle(request);
    TEST_ASSERT_EQUAL_STRING("error", reply["status"]);
    TEST_ASSERT_EQUAL_STRING(code, reply["error"]["code"]);
    TEST_ASSERT_EQUAL_UINT64(kTime, reply["time"].as<uint64_t>());
}

}  // namespace

void setUp() {
    lamp.changeOnBoard(0);
    spare.changeOnBoard(0);
    events.clear();
}

void tearDown() {}

void test_ping_echoes_id_and_time() {
    const JsonDocument reply = handle(R"({"id": 7, "cmd": "ping"})");
    TEST_ASSERT_EQUAL(7, reply["id"]);
    TEST_ASSERT_EQUAL_STRING("ok", reply["status"]);
    TEST_ASSERT_EQUAL_UINT64(kTime, reply["time"].as<uint64_t>());
}

void test_describe_lists_board_and_devices() {
    const JsonDocument reply = handle(R"({"id": 1, "cmd": "describe"})");
    TEST_ASSERT_EQUAL_STRING("test-board", reply["data"]["board"]);
    TEST_ASSERT_EQUAL_STRING("9.9.9", reply["data"]["firmware"]);
    TEST_ASSERT_EQUAL(1, reply["data"]["protocol"]);
    TEST_ASSERT_EQUAL_STRING("lamp", reply["data"]["devices"]["lamp"]["type"]);
    TEST_ASSERT_EQUAL(10, reply["data"]["devices"]["lamp"]["props"]["level"]["max"]);
    TEST_ASSERT_TRUE(reply["data"]["devices"]["spare"].is<JsonObjectConst>());
}

void test_get_all_devices() {
    lamp.changeOnBoard(3);
    const JsonDocument reply = handle(R"({"id": 2, "cmd": "get"})");
    TEST_ASSERT_EQUAL(3, reply["data"]["lamp"]["level"]);
    TEST_ASSERT_EQUAL(0, reply["data"]["spare"]["level"]);
}

void test_get_one_device() {
    const JsonDocument reply = handle(R"({"id": 3, "cmd": "get", "device": "spare"})");
    TEST_ASSERT_EQUAL_STRING("spare", reply["device"]);
    TEST_ASSERT_EQUAL(21, reply["data"]["temperature"]);
}

void test_set_changes_only_named_device() {
    const JsonDocument reply =
        handle(R"({"id": 4, "cmd": "set", "device": "spare", "data": {"level": 5}})");
    TEST_ASSERT_EQUAL_STRING("ok", reply["status"]);
    TEST_ASSERT_EQUAL(5, reply["data"]["level"]);
    TEST_ASSERT_EQUAL(5, spare.level());
    TEST_ASSERT_EQUAL(0, lamp.level());
}

void test_request_shape_errors() {
    assertError("not json", "bad_json");
    assertError(R"({"id": 1, "cmd": "ping")", "bad_json");
    assertError("[1, 2]", "bad_request");
    assertError(R"({"cmd": "ping"})", "missing_id");
    assertError(R"({"id": -1, "cmd": "ping"})", "bad_request");
    assertError(R"({"id": "7", "cmd": "ping"})", "bad_request");
    assertError(R"({"id": 1})", "bad_request");
    assertError(R"({"id": 1, "cmd": 42})", "bad_request");
    assertError(R"({"id": 1, "cmd": "foo"})", "unknown_cmd");
    assertError(R"({"id": 1, "cmd": "get", "device": 3})", "bad_request");
}

void test_device_errors() {
    assertError(R"({"id": 1, "cmd": "get", "device": "motor"})", "unknown_device");
    assertError(R"({"id": 1, "cmd": "set", "device": "motor", "data": {}})", "unknown_device");
    assertError(R"({"id": 1, "cmd": "set", "data": {"level": 1}})", "bad_request");
    assertError(R"({"id": 1, "cmd": "set", "device": "lamp"})", "bad_request");
    assertError(R"({"id": 1, "cmd": "set", "device": "lamp", "data": 5})", "bad_request");
    assertError(R"({"id": 1, "cmd": "set", "device": "lamp", "data": {"color": 1}})",
                "unknown_prop");
    assertError(R"({"id": 1, "cmd": "set", "device": "lamp", "data": {"temperature": 1}})",
                "read_only");
    assertError(R"({"id": 1, "cmd": "set", "device": "lamp", "data": {"level": 99}})",
                "bad_value");
}

void test_error_keeps_id_device_and_field() {
    const JsonDocument reply =
        handle(R"({"id": 9, "cmd": "set", "device": "lamp", "data": {"color": 1}})");
    TEST_ASSERT_EQUAL(9, reply["id"]);
    TEST_ASSERT_EQUAL_STRING("lamp", reply["device"]);
    TEST_ASSERT_EQUAL_STRING("color", reply["error"]["field"]);
    TEST_ASSERT_EQUAL_STRING("unknown property: color", reply["error"]["message"]);
}

void test_error_without_valid_id_has_no_id() {
    const JsonDocument reply = handle(R"({"id": "x", "cmd": "ping"})");
    TEST_ASSERT_TRUE(reply["id"].isNull());
}

void test_set_is_all_or_nothing() {
    const char* request =
        R"({"id": 1, "cmd": "set", "device": "lamp", "data": {"level": 4, "temperature": 1}})";
    assertError(request, "read_only");
    TEST_ASSERT_EQUAL(0, lamp.level());
}

void test_busy_error() {
    const JsonDocument reply = parse(protocol.busyError(kTime));
    TEST_ASSERT_EQUAL_STRING("busy", reply["error"]["code"]);
    TEST_ASSERT_TRUE(reply["id"].isNull());
}

void test_changed_event_has_full_state() {
    lamp.changeOnBoard(6);
    const JsonDocument event = parse(protocol.changedEvent(lamp, kTime));
    TEST_ASSERT_EQUAL_STRING("changed", event["event"]);
    TEST_ASSERT_EQUAL_STRING("lamp", event["device"]);
    TEST_ASSERT_EQUAL(6, event["data"]["level"]);
    TEST_ASSERT_EQUAL(21, event["data"]["temperature"]);
    TEST_ASSERT_TRUE(event["id"].isNull());
}

void test_change_on_board_is_forwarded() {
    lamp.changeOnBoard(2);
    TEST_ASSERT_EQUAL(1, events.size());
    TEST_ASSERT_EQUAL_STRING("lamp", events[0].c_str());
}

void test_change_by_own_set_is_not_forwarded() {
    handle(R"({"id": 1, "cmd": "set", "device": "lamp", "data": {"level": 8}})");
    TEST_ASSERT_EQUAL(8, lamp.level());
    TEST_ASSERT_EQUAL(0, events.size());
    spare.changeOnBoard(1);
    TEST_ASSERT_EQUAL(1, events.size());
}

int main() {
    devices.onChanged([](const Device& device) { events.push_back(device.name()); });
    UNITY_BEGIN();
    RUN_TEST(test_ping_echoes_id_and_time);
    RUN_TEST(test_describe_lists_board_and_devices);
    RUN_TEST(test_get_all_devices);
    RUN_TEST(test_get_one_device);
    RUN_TEST(test_set_changes_only_named_device);
    RUN_TEST(test_request_shape_errors);
    RUN_TEST(test_device_errors);
    RUN_TEST(test_error_keeps_id_device_and_field);
    RUN_TEST(test_error_without_valid_id_has_no_id);
    RUN_TEST(test_set_is_all_or_nothing);
    RUN_TEST(test_busy_error);
    RUN_TEST(test_changed_event_has_full_state);
    RUN_TEST(test_change_on_board_is_forwarded);
    RUN_TEST(test_change_by_own_set_is_not_forwarded);
    return UNITY_END();
}
