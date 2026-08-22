#ifndef ISAACCOYOTE_APP_COYOTE_WEBSOCKET_DEVICE_H
#define ISAACCOYOTE_APP_COYOTE_WEBSOCKET_DEVICE_H

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "common.h"

namespace app::coyote::websocket::model
{
    struct ComfortLimit
    {
        ComfortLimitMode mode{};
        double comfort_max{};
        double absolute_max{};
        bool overheat{};
        double overheat_percent{};
        bool auto_increment{};
        double auto_increment_max{};
        AutoIncrementScope auto_increment_scope{};
        double total_increment{};
    };

    struct ComfortLimitPatch
    {
        std::optional<ComfortLimitMode> mode;
        std::optional<double> comfort_max;
        std::optional<double> absolute_max;
        std::optional<bool> overheat;
        std::optional<double> overheat_percent;
        std::optional<bool> auto_increment;
        std::optional<double> auto_increment_max;
        std::optional<AutoIncrementScope> auto_increment_scope;
        std::optional<double> total_increment;
    };

    struct ChannelState
    {
        bool is_muted{};
        double warm_up_scale{};
        double intensity_max{};
        ComfortLimit comfort_limit;
    };

    struct ChannelStatePatch
    {
        std::optional<bool> is_muted;
        std::optional<double> warm_up_scale;
        std::optional<double> intensity_max;
        std::optional<ComfortLimitPatch> comfort_limit;
    };

    struct SlotState
    {
        std::optional<MarkLight> mark_light;
        bool has_device{};
        std::optional<ChannelState> channel_a;
        std::optional<ChannelState> channel_b;
        nlohmann::json extra = nlohmann::json::object();
    };

    struct SlotStatePatch
    {
        // Outer optional means changed; inner optional represents JSON null.
        std::optional<std::optional<MarkLight>> mark_light;
        std::optional<bool> has_device;
        std::optional<ChannelStatePatch> channel_a;
        std::optional<ChannelStatePatch> channel_b;
        nlohmann::json extra = nlohmann::json::object();
    };

    struct CoyoteProps
    {
        double power{};
        double version{};
        double label{};
        double intensity_a{};
        double intensity_b{};
        std::string connect_state;
        std::optional<ChannelOutputStatus> channel_a_status;
        std::optional<ChannelOutputStatus> channel_b_status;
        std::optional<std::string> update_value;
        nlohmann::json extra = nlohmann::json::object();
    };

    struct CoyotePropsPatch
    {
        std::optional<double> power;
        std::optional<double> version;
        std::optional<double> label;
        std::optional<double> intensity_a;
        std::optional<double> intensity_b;
        std::optional<std::string> connect_state;
        std::optional<ChannelOutputStatus> channel_a_status;
        std::optional<ChannelOutputStatus> channel_b_status;
        std::optional<std::string> update_value;
        nlohmann::json extra = nlohmann::json::object();
    };

    struct RemoteDevice
    {
        std::optional<int> id;
        SlotId slot_id;
        std::string name;
        std::string type;
        std::optional<CoyoteProps> props;
        std::optional<SlotState> slot_state;
    };

    struct SlotPatch
    {
        SlotId slot_id;
        std::optional<CoyotePropsPatch> props;
        std::optional<SlotStatePatch> slot_state;
    };

    struct DevicesGetResult
    {
        std::vector<RemoteDevice> devices;
    };

    struct DevicesSnapshotEvent
    {
        std::vector<RemoteDevice> devices;
    };

    struct DevicesPatchEvent
    {
        std::vector<RemoteDevice> added;
        std::vector<SlotId> removed;
    };

    struct SlotsPatchEvent
    {
        std::vector<SlotPatch> slots;
    };

    struct CustomActionEvent
    {
        std::uint8_t action{};
    };

#define ISAACCOYOTE_DECLARE_JSON(Type) \
    void to_json(nlohmann::json& j, const Type& value); \
    void from_json(const nlohmann::json& j, Type& value)

    ISAACCOYOTE_DECLARE_JSON(ComfortLimit);
    ISAACCOYOTE_DECLARE_JSON(ComfortLimitPatch);
    ISAACCOYOTE_DECLARE_JSON(ChannelState);
    ISAACCOYOTE_DECLARE_JSON(ChannelStatePatch);
    ISAACCOYOTE_DECLARE_JSON(SlotState);
    ISAACCOYOTE_DECLARE_JSON(SlotStatePatch);
    ISAACCOYOTE_DECLARE_JSON(CoyoteProps);
    ISAACCOYOTE_DECLARE_JSON(CoyotePropsPatch);
    ISAACCOYOTE_DECLARE_JSON(RemoteDevice);
    ISAACCOYOTE_DECLARE_JSON(SlotPatch);
    ISAACCOYOTE_DECLARE_JSON(DevicesGetResult);
    ISAACCOYOTE_DECLARE_JSON(DevicesSnapshotEvent);
    ISAACCOYOTE_DECLARE_JSON(DevicesPatchEvent);
    ISAACCOYOTE_DECLARE_JSON(SlotsPatchEvent);
    ISAACCOYOTE_DECLARE_JSON(CustomActionEvent);

#undef ISAACCOYOTE_DECLARE_JSON
}

#endif // ISAACCOYOTE_APP_COYOTE_WEBSOCKET_DEVICE_H
