#ifndef ISAACCOYOTE_APP_COYOTE_WEBSOCKET_COMMON_H
#define ISAACCOYOTE_APP_COYOTE_WEBSOCKET_COMMON_H

#include <cstdint>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace app::coyote::websocket::model
{
    using ClientId = std::string;
    using RequestId = std::string;
    using SlotId = std::string;
    using Timestamp = std::int64_t;

    enum class RpcMethod
    {
        DevicesGet,
        Ping,
        DeviceOperate,
        DeviceOperateClear,
    };

    enum class EventType
    {
        DevicesSnapshot,
        DevicesPatch,
        SlotsPatch,
        CustomAction,
    };

    enum class ActionType : std::uint8_t
    {
        AppendPulseData = 0,
        AppendPulse = 1,
        ClearPulse = 2,
        AddIntensity = 3,
        SetTempIntensity = 4,
        SetMute = 5,
        SetVar = 6,
        SetIntensity = 7,
    };

    enum class Channel : std::uint8_t
    {
        A = 0,
        B = 1,
    };

    enum class Priority : std::uint8_t
    {
        Low = 0,
        Normal = 1,
        High = 2,
    };

    enum class TaskEndReason
    {
        Completed,
        Cleared,
        Replaced,
        Cancelled,
    };

    enum class MarkLight
    {
        Yellow,
        Green,
        Red,
        Purple,
        Blue,
        Cyan,
    };

    enum class ComfortLimitMode
    {
        Simple,
        Complex,
    };

    enum class AutoIncrementScope : std::uint8_t
    {
        ComfortOnly = 1,
        AbsoluteOnly = 2,
        Both = 3,
    };

    enum class ChannelOutputStatus : std::uint8_t
    {
        NoOutput = 0,
        OpenCircuit = 1,
        Normal = 2,
        Damaged = 3,
        Disabled = 4,
    };

    void to_json(nlohmann::json& j, RpcMethod value);
    void from_json(const nlohmann::json& j, RpcMethod& value);
    void to_json(nlohmann::json& j, EventType value);
    void from_json(const nlohmann::json& j, EventType& value);
    void to_json(nlohmann::json& j, ActionType value);
    void from_json(const nlohmann::json& j, ActionType& value);
    void to_json(nlohmann::json& j, Channel value);
    void from_json(const nlohmann::json& j, Channel& value);
    void to_json(nlohmann::json& j, Priority value);
    void from_json(const nlohmann::json& j, Priority& value);
    void to_json(nlohmann::json& j, TaskEndReason value);
    void from_json(const nlohmann::json& j, TaskEndReason& value);
    void to_json(nlohmann::json& j, MarkLight value);
    void from_json(const nlohmann::json& j, MarkLight& value);
    void to_json(nlohmann::json& j, ComfortLimitMode value);
    void from_json(const nlohmann::json& j, ComfortLimitMode& value);
    void to_json(nlohmann::json& j, AutoIncrementScope value);
    void from_json(const nlohmann::json& j, AutoIncrementScope& value);
    void to_json(nlohmann::json& j, ChannelOutputStatus value);
    void from_json(const nlohmann::json& j, ChannelOutputStatus& value);
}

#endif // ISAACCOYOTE_APP_COYOTE_WEBSOCKET_COMMON_H
