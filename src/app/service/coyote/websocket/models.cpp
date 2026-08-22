#include <array>
#include <string_view>
#include <type_traits>

#include "app/service/coyote/websocket/models/frame.h"

namespace app::coyote::websocket::model
{
    namespace
    {
        using Json = nlohmann::json;
        using namespace std::string_view_literals;

        template <typename Enum, std::size_t Size>
        void enum_to_json(Json& j, const Enum value,
                          const std::array<std::pair<Enum, std::string_view>, Size>& values) {
            for (const auto& [candidate, text] : values)
                if (candidate == value) {
                    j = text;
                    return;
                }
            throw Json::other_error::create(501, "invalid enum value", &j);
        }

        template <typename Enum, std::size_t Size>
        void enum_from_json(const Json& j, Enum& value,
                            const std::array<std::pair<Enum, std::string_view>, Size>& values) {
            const auto text = j.get<std::string>();
            for (const auto& [candidate, expected] : values)
                if (text == expected) {
                    value = candidate;
                    return;
                }
            throw Json::other_error::create(501, "unknown enum value: " + text, &j);
        }

        template <typename Enum>
        void numeric_enum_to_json(Json& j, const Enum value) {
            j = static_cast<std::underlying_type_t<Enum>>(value);
        }

        template <typename Enum, std::size_t Size>
        void numeric_enum_from_json(const Json& j, Enum& value, const std::array<Enum, Size>& values) {
            const auto raw = j.get<std::underlying_type_t<Enum>>();
            for (const auto candidate : values)
                if (static_cast<std::underlying_type_t<Enum>>(candidate) == raw) {
                    value = candidate;
                    return;
                }
            throw Json::other_error::create(501, "unknown numeric enum value", &j);
        }

        template <typename T>
        void put_optional(Json& j, const char* key, const std::optional<T>& value) {
            if (value)
                j[key] = *value;
        }

        template <typename T>
        void get_optional(const Json& j, const char* key, std::optional<T>& value) {
            const auto it = j.find(key);
            if (it == j.end() || it->is_null())
                value.reset();
            else
                value = it->template get<T>();
        }

        Json with_extra(const Json& extra) {
            return extra.is_object() ? extra : Json::object();
        }

        Json unknown_fields(const Json& j, const std::initializer_list<std::string_view> known) {
            Json result = Json::object();
            for (auto it = j.begin(); it != j.end(); ++it) {
                bool is_known = false;
                for (const auto key : known)
                    if (it.key() == key) {
                        is_known = true;
                        break;
                    }
                if (!is_known)
                    result[it.key()] = it.value();
            }
            return result;
        }

        void put_operate_common(Json& j, const OperateCommon& value) {
            j["s"] = value.slot_id;
            j["c"] = value.channel;
            put_optional(j, "p", value.priority);
            put_optional(j, "d", value.duration_ms);
            put_optional(j, "im", value.immediate);
        }

        void get_operate_common(const Json& j, OperateCommon& value) {
            j.at("s").get_to(value.slot_id);
            j.at("c").get_to(value.channel);
            if (const auto it = j.find("p"); it != j.end() && !it->is_null())
                it->get_to(value.priority);
            else
                value.priority = Priority::Normal;
            value.duration_ms = j.value("d", std::uint64_t{});
            get_optional(j, "im", value.immediate);
        }

        template <typename T>
        Json object_with(const char* key, const T& value) {
            return Json{{key, value}};
        }
    }

    void to_json(Json& j, const RpcMethod value) {
        enum_to_json(j, value, std::array{
                         std::pair{RpcMethod::DevicesGet, "devices.get"sv}, std::pair{RpcMethod::Ping, "ping"sv},
                         std::pair{RpcMethod::DeviceOperate, "device.op"sv},
                         std::pair{RpcMethod::DeviceOperateClear, "device.op.clear"sv}
                     });
    }

    void from_json(const Json& j, RpcMethod& value) {
        enum_from_json(j, value, std::array{
                           std::pair{RpcMethod::DevicesGet, "devices.get"sv}, std::pair{RpcMethod::Ping, "ping"sv},
                           std::pair{RpcMethod::DeviceOperate, "device.op"sv},
                           std::pair{RpcMethod::DeviceOperateClear, "device.op.clear"sv}
                       });
    }

    void to_json(Json& j, const EventType value) {
        enum_to_json(j, value, std::array{
                         std::pair{EventType::DevicesSnapshot, "devices.snapshot"sv},
                         std::pair{EventType::DevicesPatch, "devices.patch"sv},
                         std::pair{EventType::SlotsPatch, "slots.patch"sv},
                         std::pair{EventType::CustomAction, "custom.action"sv}
                     });
    }

    void from_json(const Json& j, EventType& value) {
        enum_from_json(j, value, std::array{
                           std::pair{EventType::DevicesSnapshot, "devices.snapshot"sv},
                           std::pair{EventType::DevicesPatch, "devices.patch"sv},
                           std::pair{EventType::SlotsPatch, "slots.patch"sv},
                           std::pair{EventType::CustomAction, "custom.action"sv}
                       });
    }

    void to_json(Json& j, const ActionType value) { numeric_enum_to_json(j, value); }

    void from_json(const Json& j, ActionType& value) {
        numeric_enum_from_json(j, value, std::array{
                                   ActionType::AppendPulseData, ActionType::AppendPulse, ActionType::ClearPulse,
                                   ActionType::AddIntensity,
                                   ActionType::SetTempIntensity, ActionType::SetMute, ActionType::SetVar,
                                   ActionType::SetIntensity
                               });
    }

    void to_json(Json& j, const Channel value) { numeric_enum_to_json(j, value); }

    void from_json(const Json& j, Channel& value) {
        numeric_enum_from_json(j, value, std::array{Channel::A, Channel::B});
    }

    void to_json(Json& j, const Priority value) { numeric_enum_to_json(j, value); }

    void from_json(const Json& j, Priority& value) {
        numeric_enum_from_json(j, value, std::array{Priority::Low, Priority::Normal, Priority::High});
    }

    void to_json(Json& j, const AutoIncrementScope value) { numeric_enum_to_json(j, value); }

    void from_json(const Json& j, AutoIncrementScope& value) {
        numeric_enum_from_json(j, value, std::array{
                                   AutoIncrementScope::ComfortOnly, AutoIncrementScope::AbsoluteOnly,
                                   AutoIncrementScope::Both
                               });
    }

    void to_json(Json& j, const ChannelOutputStatus value) { numeric_enum_to_json(j, value); }

    void from_json(const Json& j, ChannelOutputStatus& value) {
        numeric_enum_from_json(j, value, std::array{
                                   ChannelOutputStatus::NoOutput, ChannelOutputStatus::OpenCircuit,
                                   ChannelOutputStatus::Normal,
                                   ChannelOutputStatus::Damaged, ChannelOutputStatus::Disabled
                               });
    }

    #define STRING_ENUM_JSON(Type, ...) \
    void to_json(Json& j, const Type value) { enum_to_json(j, value, std::array{__VA_ARGS__}); } \
    void from_json(const Json& j, Type& value) { enum_from_json(j, value, std::array{__VA_ARGS__}); }

    STRING_ENUM_JSON(TaskEndReason,
                     std::pair{TaskEndReason::Completed, "completed"sv}, std::pair{TaskEndReason::Cleared, "cleared"sv},
                     std::pair{TaskEndReason::Replaced, "replaced"sv}, std::pair{TaskEndReason::Cancelled,
                     "cancelled"sv})
    STRING_ENUM_JSON(MarkLight,
                     std::pair{MarkLight::Yellow, "yellow"sv}, std::pair{MarkLight::Green, "green"sv},
                     std::pair{MarkLight::Red, "red"sv}, std::pair{MarkLight::Purple, "purple"sv},
                     std::pair{MarkLight::Blue, "blue"sv}, std::pair{MarkLight::Cyan, "cyan"sv})
    STRING_ENUM_JSON(ComfortLimitMode,
                     std::pair{ComfortLimitMode::Simple, "simple"sv}, std::pair{ComfortLimitMode::Complex, "complex"sv})
    #undef STRING_ENUM_JSON

    #define REQUIRED_FIELD(key, member) j.at(key).get_to(value.member)
    #define OPTIONAL_FIELD(key, member) get_optional(j, key, value.member)
    #define PUT_OPTIONAL_FIELD(key, member) put_optional(j, key, value.member)

    void to_json(Json& j, const ComfortLimit& value) {
        j = {
            {"mode", value.mode}, {"comfortMax", value.comfort_max}, {"absoluteMax", value.absolute_max},
            {"overheat", value.overheat}, {"overheatPercent", value.overheat_percent},
            {"autoIncr", value.auto_increment}, {"autoIncrMax", value.auto_increment_max},
            {"autoIncrScope", value.auto_increment_scope}, {"totalIncr", value.total_increment}
        };
    }

    void from_json(const Json& j, ComfortLimit& value) {
        REQUIRED_FIELD("mode", mode);
        REQUIRED_FIELD("comfortMax", comfort_max);
        REQUIRED_FIELD("absoluteMax", absolute_max);
        REQUIRED_FIELD("overheat", overheat);
        REQUIRED_FIELD("overheatPercent", overheat_percent);
        j.at("autoIncr").get_to(value.auto_increment);
        j.at("autoIncrMax").get_to(value.auto_increment_max);
        j.at("autoIncrScope").get_to(value.auto_increment_scope);
        j.at("totalIncr").get_to(value.total_increment);
    }

    void to_json(Json& j, const ComfortLimitPatch& value) {
        j = Json::object();
        PUT_OPTIONAL_FIELD("mode", mode);
        PUT_OPTIONAL_FIELD("comfortMax", comfort_max);
        PUT_OPTIONAL_FIELD("absoluteMax", absolute_max);
        PUT_OPTIONAL_FIELD("overheat", overheat);
        PUT_OPTIONAL_FIELD("overheatPercent", overheat_percent);
        PUT_OPTIONAL_FIELD("autoIncr", auto_increment);
        PUT_OPTIONAL_FIELD("autoIncrMax", auto_increment_max);
        PUT_OPTIONAL_FIELD("autoIncrScope", auto_increment_scope);
        PUT_OPTIONAL_FIELD("totalIncr", total_increment);
    }

    void from_json(const Json& j, ComfortLimitPatch& value) {
        OPTIONAL_FIELD("mode", mode);
        OPTIONAL_FIELD("comfortMax", comfort_max);
        OPTIONAL_FIELD("absoluteMax", absolute_max);
        OPTIONAL_FIELD("overheat", overheat);
        OPTIONAL_FIELD("overheatPercent", overheat_percent);
        OPTIONAL_FIELD("autoIncr", auto_increment);
        OPTIONAL_FIELD("autoIncrMax", auto_increment_max);
        OPTIONAL_FIELD("autoIncrScope", auto_increment_scope);
        OPTIONAL_FIELD("totalIncr", total_increment);
    }

    void to_json(Json& j, const ChannelState& value) {
        j = {
            {"isMuted", value.is_muted}, {"warmUpScale", value.warm_up_scale},
            {"intensityMax", value.intensity_max}, {"comfortLimit", value.comfort_limit}
        };
    }

    void from_json(const Json& j, ChannelState& value) {
        REQUIRED_FIELD("isMuted", is_muted);
        REQUIRED_FIELD("warmUpScale", warm_up_scale);
        REQUIRED_FIELD("intensityMax", intensity_max);
        REQUIRED_FIELD("comfortLimit", comfort_limit);
    }

    void to_json(Json& j, const ChannelStatePatch& value) {
        j = Json::object();
        PUT_OPTIONAL_FIELD("isMuted", is_muted);
        PUT_OPTIONAL_FIELD("warmUpScale", warm_up_scale);
        PUT_OPTIONAL_FIELD("intensityMax", intensity_max);
        PUT_OPTIONAL_FIELD("comfortLimit", comfort_limit);
    }

    void from_json(const Json& j, ChannelStatePatch& value) {
        OPTIONAL_FIELD("isMuted", is_muted);
        OPTIONAL_FIELD("warmUpScale", warm_up_scale);
        OPTIONAL_FIELD("intensityMax", intensity_max);
        OPTIONAL_FIELD("comfortLimit", comfort_limit);
    }

    void to_json(Json& j, const SlotState& value) {
        j = with_extra(value.extra);
        PUT_OPTIONAL_FIELD("markLight", mark_light);
        j["hasDevice"] = value.has_device;
        PUT_OPTIONAL_FIELD("channelA", channel_a);
        PUT_OPTIONAL_FIELD("channelB", channel_b);
    }

    void from_json(const Json& j, SlotState& value) {
        OPTIONAL_FIELD("markLight", mark_light);
        REQUIRED_FIELD("hasDevice", has_device);
        OPTIONAL_FIELD("channelA", channel_a);
        OPTIONAL_FIELD("channelB", channel_b);
        value.extra = unknown_fields(j, {"markLight", "hasDevice", "channelA", "channelB"});
    }

    void to_json(Json& j, const SlotStatePatch& value) {
        j = with_extra(value.extra);
        if (value.mark_light) j["markLight"] = *value.mark_light ? Json(**value.mark_light) : Json(nullptr);
        PUT_OPTIONAL_FIELD("hasDevice", has_device);
        PUT_OPTIONAL_FIELD("channelA", channel_a);
        PUT_OPTIONAL_FIELD("channelB", channel_b);
    }

    void from_json(const Json& j, SlotStatePatch& value) {
        if (const auto it = j.find("markLight"); it == j.end()) value.mark_light.reset();
        else if (it->is_null()) value.mark_light = std::optional<MarkLight>{};
        else value.mark_light = it->get<MarkLight>();
        OPTIONAL_FIELD("hasDevice", has_device);
        OPTIONAL_FIELD("channelA", channel_a);
        OPTIONAL_FIELD("channelB", channel_b);
        value.extra = unknown_fields(j, {"markLight", "hasDevice", "channelA", "channelB"});
    }

    void to_json(Json& j, const CoyoteProps& value) {
        j = with_extra(value.extra);
        j.update({
            {"power", value.power}, {"version", value.version}, {"label", value.label},
            {"intensityA", value.intensity_a}, {"intensityB", value.intensity_b}, {"connectState", value.connect_state}
        });
        PUT_OPTIONAL_FIELD("channelAStatus", channel_a_status);
        PUT_OPTIONAL_FIELD("channelBStatus", channel_b_status);
        PUT_OPTIONAL_FIELD("updateValue", update_value);
    }

    void from_json(const Json& j, CoyoteProps& value) {
        REQUIRED_FIELD("power", power);
        REQUIRED_FIELD("version", version);
        REQUIRED_FIELD("label", label);
        REQUIRED_FIELD("intensityA", intensity_a);
        REQUIRED_FIELD("intensityB", intensity_b);
        REQUIRED_FIELD("connectState", connect_state);
        OPTIONAL_FIELD("channelAStatus", channel_a_status);
        OPTIONAL_FIELD("channelBStatus", channel_b_status);
        OPTIONAL_FIELD("updateValue", update_value);
        value.extra = unknown_fields(j, {
                                         "power", "version", "label", "intensityA", "intensityB", "connectState",
                                         "channelAStatus", "channelBStatus", "updateValue"
                                     });
    }

    void to_json(Json& j, const CoyotePropsPatch& value) {
        j = with_extra(value.extra);
        PUT_OPTIONAL_FIELD("power", power);
        PUT_OPTIONAL_FIELD("version", version);
        PUT_OPTIONAL_FIELD("label", label);
        PUT_OPTIONAL_FIELD("intensityA", intensity_a);
        PUT_OPTIONAL_FIELD("intensityB", intensity_b);
        PUT_OPTIONAL_FIELD("connectState", connect_state);
        PUT_OPTIONAL_FIELD("channelAStatus", channel_a_status);
        PUT_OPTIONAL_FIELD("channelBStatus", channel_b_status);
        PUT_OPTIONAL_FIELD("updateValue", update_value);
    }

    void from_json(const Json& j, CoyotePropsPatch& value) {
        OPTIONAL_FIELD("power", power);
        OPTIONAL_FIELD("version", version);
        OPTIONAL_FIELD("label", label);
        OPTIONAL_FIELD("intensityA", intensity_a);
        OPTIONAL_FIELD("intensityB", intensity_b);
        OPTIONAL_FIELD("connectState", connect_state);
        OPTIONAL_FIELD("channelAStatus", channel_a_status);
        OPTIONAL_FIELD("channelBStatus", channel_b_status);
        OPTIONAL_FIELD("updateValue", update_value);
        value.extra = unknown_fields(j, {
                                         "power", "version", "label", "intensityA", "intensityB", "connectState",
                                         "channelAStatus", "channelBStatus", "updateValue"
                                     });
    }

    void to_json(Json& j, const RemoteDevice& value) {
        j = Json::object();
        PUT_OPTIONAL_FIELD("id", id);
        j["slotId"] = value.slot_id;
        j["name"] = value.name;
        j["type"] = value.type;
        PUT_OPTIONAL_FIELD("props", props);
        PUT_OPTIONAL_FIELD("slotState", slot_state);
    }

    void from_json(const Json& j, RemoteDevice& value) {
        OPTIONAL_FIELD("id", id);
        REQUIRED_FIELD("slotId", slot_id);
        REQUIRED_FIELD("name", name);
        REQUIRED_FIELD("type", type);
        OPTIONAL_FIELD("props", props);
        OPTIONAL_FIELD("slotState", slot_state);
    }

    void to_json(Json& j, const SlotPatch& value) {
        j = object_with("slotId", value.slot_id);
        PUT_OPTIONAL_FIELD("props", props);
        PUT_OPTIONAL_FIELD("slotState", slot_state);
    }

    void from_json(const Json& j, SlotPatch& value) {
        REQUIRED_FIELD("slotId", slot_id);
        OPTIONAL_FIELD("props", props);
        OPTIONAL_FIELD("slotState", slot_state);
    }

    void to_json(Json& j, const DevicesGetResult& value) { j = object_with("devices", value.devices); }
    void from_json(const Json& j, DevicesGetResult& value) { REQUIRED_FIELD("devices", devices); }
    void to_json(Json& j, const DevicesSnapshotEvent& value) { j = object_with("devices", value.devices); }
    void from_json(const Json& j, DevicesSnapshotEvent& value) { REQUIRED_FIELD("devices", devices); }
    void to_json(Json& j, const DevicesPatchEvent& value) { j = {{"added", value.added}, {"removed", value.removed}}; }

    void from_json(const Json& j, DevicesPatchEvent& value) {
        REQUIRED_FIELD("added", added);
        REQUIRED_FIELD("removed", removed);
    }

    void to_json(Json& j, const SlotsPatchEvent& value) { j = object_with("slots", value.slots); }
    void from_json(const Json& j, SlotsPatchEvent& value) { REQUIRED_FIELD("slots", slots); }
    void to_json(Json& j, const CustomActionEvent& value) { j = object_with("action", value.action); }
    void from_json(const Json& j, CustomActionEvent& value) { REQUIRED_FIELD("action", action); }

    void to_json(Json& j, const DeviceOperate& value) {
        std::visit([&j](const auto& operate)
        {
            j = Json::object();
            put_operate_common(j, operate.common);
            using T = std::decay_t<decltype(operate)>;
            if constexpr (std::is_same_v<T, AppendPulseDataOperate>) {
                j["t"] = ActionType::AppendPulseData;
                j["v"] = operate.value;
                put_optional(j, "seq", operate.sequence);
            }
            else {
                if constexpr (std::is_same_v<T, AddIntensityOperate>) j["t"] = ActionType::AddIntensity;
                else if constexpr (std::is_same_v<T, SetTempIntensityOperate>) j["t"] = ActionType::SetTempIntensity;
                else if constexpr (std::is_same_v<T, SetMuteOperate>) j["t"] = ActionType::SetMute;
                else j["t"] = ActionType::SetIntensity;
                j["v"] = operate.value;
            }
        }, value);
    }

    void from_json(const Json& j, DeviceOperate& value) {
        ActionType type{};
        j.at("t").get_to(type);
        if (type == ActionType::AppendPulseData) {
            AppendPulseDataOperate operate;
            get_operate_common(j, operate.common);
            get_optional(j, "seq", operate.sequence);
            operate.value = j.at("v").get<PulseTrack>();
            value = std::move(operate);
            return;
        }

        const auto parse = [&j, &value]<typename T>()
        {
            T operate;
            get_operate_common(j, operate.common);
            j.at("v").get_to(operate.value);
            value = std::move(operate);
        };
        switch (type) {
            case ActionType::AddIntensity: parse.template operator()<AddIntensityOperate>();
                return;
            case ActionType::SetTempIntensity: parse.template operator()<SetTempIntensityOperate>();
                return;
            case ActionType::SetMute: parse.template operator()<SetMuteOperate>();
                return;
            case ActionType::SetIntensity: parse.template operator()<SetIntensityOperate>();
                return;
            default: throw Json::other_error::create(501, "unsupported device operation type", &j);
        }
    }

    void to_json(Json& j, const ClearOperate& value) {
        j = Json::object();
        put_optional(j, "s", value.slot_id);
        put_optional(j, "c", value.channel);
    }

    void from_json(const Json& j, ClearOperate& value) {
        get_optional(j, "s", value.slot_id);
        get_optional(j, "c", value.channel);
    }

    void to_json(Json& j, const DeviceOperateResult& value) {
        j = {{"type", value.type}, {"reason", value.reason}};
        put_optional(j, "slotId", value.slot_id);
        put_optional(j, "channel", value.channel);
    }

    void from_json(const Json& j, DeviceOperateResult& value) {
        REQUIRED_FIELD("type", type);
        REQUIRED_FIELD("reason", reason);
        OPTIONAL_FIELD("slotId", slot_id);
        OPTIONAL_FIELD("channel", channel);
    }

    void to_json(Json& j, const AppEvent& value) {
        j = {{"t", "ev"}, {"ev", value.event}};
        std::visit([&j](const auto& data) { j.update(Json(data)); }, value.data);
    }

    void from_json(const Json& j, AppEvent& value) {
        if (j.at("t") != "ev") throw Json::other_error::create(501, "expected V4 event message", &j);
        j.at("ev").get_to(value.event);
        switch (value.event) {
            case EventType::DevicesSnapshot: value.data = j.get<DevicesSnapshotEvent>();
                return;
            case EventType::DevicesPatch: value.data = j.get<DevicesPatchEvent>();
                return;
            case EventType::SlotsPatch: value.data = j.get<SlotsPatchEvent>();
                return;
            case EventType::CustomAction: value.data = j.get<CustomActionEvent>();
                return;
        }
        throw Json::other_error::create(501, "unsupported event type", &j);
    }

    void to_json(Json& j, const RpcRequest& value) {
        j = {{"t", "req"}, {"reqId", value.request_id}, {"m", value.method}};
        std::visit([&j](const auto& data)
        {
            using T = std::decay_t<decltype(data)>;
            if constexpr (std::is_same_v<T, EmptyData>) return;
            else j["data"] = data;
        }, value.data);
    }

    void from_json(const Json& j, RpcRequest& value) {
        if (j.at("t") != "req") throw Json::other_error::create(501, "expected V4 RPC request", &j);
        j.at("reqId").get_to(value.request_id);
        j.at("m").get_to(value.method);
        switch (value.method) {
            case RpcMethod::DevicesGet:
            case RpcMethod::Ping: value.data = EmptyData{};
                return;
            case RpcMethod::DeviceOperate: value.data = j.at("data").get<DeviceOperate>();
                return;
            case RpcMethod::DeviceOperateClear:
                value.data = j.contains("data") ? j.at("data").get<ClearOperate>() : ClearOperate{};
                return;
        }
        throw Json::other_error::create(501, "unsupported RPC method", &j);
    }

    void to_json(Json& j, const RpcResponse& value) {
        j = {{"t", "resp"}, {"reqId", value.request_id}};
        if (value.result)
            std::visit([&j](const auto& result)
            {
                using T = std::decay_t<decltype(result)>;
                if constexpr (std::is_same_v<T, EmptyResult>) j["result"] = Json::object();
                else j["result"] = result;
            }, *value.result);
        put_optional(j, "error", value.error);
    }

    void from_json(const Json& j, RpcResponse& value) {
        if (j.at("t") != "resp") throw Json::other_error::create(501, "expected V4 RPC response", &j);
        j.at("reqId").get_to(value.request_id);
        get_optional(j, "error", value.error);

        const auto result = j.find("result");
        if (result == j.end() || result->is_null()) {
            value.result.reset();
            return;
        }

        if (result->is_number_integer()) {
            value.result = result->get<Timestamp>();
            return;
        }

        if (!result->is_object())
            throw Json::type_error::create(302, "unsupported RPC result format", &*result);

        if (result->contains("devices")) {
            value.result = result->get<DevicesGetResult>();
            return;
        }

        if (result->contains("type")) {
            value.result = result->get<DeviceOperateResult>();
            return;
        }

        if (result->empty()) {
            value.result = EmptyResult{};
            return;
        }

        throw Json::other_error::create(501, "unsupported RPC result object", &*result);
    }

    void to_json(Json& j, const AppMessage& value) {
        std::visit([&j](const auto& message) { j = message; }, value);
    }

    void from_json(const Json& j, AppMessage& value) {
        const auto type = j.at("t").get<std::string>();
        if (type == "req") value = j.get<RpcRequest>();
        else if (type == "resp") value = j.get<RpcResponse>();
        else if (type == "ev") value = j.get<AppEvent>();
        else throw Json::other_error::create(501, "unknown V4 app message type: " + type, &j);
    }

    void to_json(Json& j, const ServerFrame& value) {
        std::visit([&j](const auto& frame)
        {
            using T = std::decay_t<decltype(frame)>;
            if constexpr (std::is_same_v<T, HelloFrame>) j = {{"type", "hello"}, {"clientId", frame.client_id}};
            else if constexpr (std::is_same_v<T, ClientAttachedFrame>) j = {
                {"type", "client_attached"}, {"clientId", frame.client_id}
            };
            else if constexpr (std::is_same_v<T, ClientDisconnectedFrame>) j = {
                {"type", "client_disconnected"}, {"clientId", frame.client_id}
            };
            else if constexpr (std::is_same_v<T, ControllerAttachedFrame>) j = {
                {"type", "controller_attached"}, {"clientId", frame.client_id}
            };
            else if constexpr (std::is_same_v<T, ControllerDisconnectedFrame>) j = {
                {"type", "controller_disconnected"}, {"clientId", frame.client_id}
            };
            else if constexpr (std::is_same_v<T, HeartbeatFrame>) j = {{"type", "heartbeat"}};
            else if constexpr (std::is_same_v<T, PingFrame>) j = {{"type", "ping"}};
            else if constexpr (std::is_same_v<T, PongFrame>) {
                j = {{"type", "pong"}};
                put_optional(j, "ts", frame.timestamp);
            }
            else if constexpr (std::is_same_v<T, MessageFrame>) j = {
                {"type", "message"}, {"clientId", frame.client_id}, {"data", frame.data}
            };
            else if constexpr (std::is_same_v<T, IdleTimeoutFrame>) j = {{"type", "idle_timeout"}};
            else j = {{"type", "error"}, {"error", frame.error}, {"data", frame.data}};
        }, value);
    }

    void from_json(const Json& j, ServerFrame& value) {
        const auto type = j.at("type").get<std::string>();
        if (type == "hello") value = HelloFrame{j.at("clientId").get<ClientId>()};
        else if (type == "client_attached")
            value = ClientAttachedFrame{j.at("clientId").get<ClientId>()};
        else if (type == "client_disconnected")
            value = ClientDisconnectedFrame{j.at("clientId").get<ClientId>()};
        else if (type == "controller_attached")
            value = ControllerAttachedFrame{j.at("clientId").get<ClientId>()};
        else if (type == "controller_disconnected")
            value = ControllerDisconnectedFrame{
            j.at("clientId").get<ClientId>()
        };
        else if (type == "heartbeat") value = HeartbeatFrame{};
        else if (type == "ping") value = PingFrame{};
        else if (type == "pong") {
            PongFrame frame;
            get_optional(j, "ts", frame.timestamp);
            value = frame;
        }
        else if (type == "message") value = MessageFrame{
            j.at("clientId").get<ClientId>(), j.at("data").get<AppMessage>()
        };
        else if (type == "idle_timeout") value = IdleTimeoutFrame{};
        else if (type == "error") value = ErrorFrame{j.at("error").get<std::string>(), j.value("data", Json::object())};
        else throw Json::other_error::create(501, "unknown server frame type: " + type, &j);
    }

    void to_json(Json& j, const CloseInfo& value) { j = {{"code", value.code}, {"reason", value.reason}}; }

    void from_json(const Json& j, CloseInfo& value) {
        REQUIRED_FIELD("code", code);
        REQUIRED_FIELD("reason", reason);
    }

    #undef REQUIRED_FIELD
    #undef OPTIONAL_FIELD
    #undef PUT_OPTIONAL_FIELD
}
