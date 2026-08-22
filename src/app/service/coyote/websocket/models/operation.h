#ifndef ISAACCOYOTE_APP_COYOTE_WEBSOCKET_OPERATION_H
#define ISAACCOYOTE_APP_COYOTE_WEBSOCKET_OPERATION_H

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "common.h"

namespace app::coyote::websocket::model
{
    using PulseTrack = std::vector<std::string>;

    struct OperateCommon {
        SlotId slot_id;
        Channel channel{};
        std::optional<Priority> priority = std::nullopt;
        std::optional<std::int64_t> duration_ms = std::nullopt;
        std::optional<bool> immediate = std::nullopt;
    };

    struct AppendPulseDataOperate {
        OperateCommon common;
        PulseTrack value;
        std::optional<std::int64_t> sequence;
    };

    struct AddIntensityOperate {
        OperateCommon common;
        int value{};
    };

    struct SetTempIntensityOperate {
        OperateCommon common;
        int value{};
    };

    struct SetMuteOperate {
        OperateCommon common;
        bool value{};
    };

    struct SetIntensityOperate {
        OperateCommon common;
        int value{};
    };

    using DeviceOperate = std::variant<AppendPulseDataOperate,
                                       AddIntensityOperate,
                                       SetTempIntensityOperate,
                                       SetMuteOperate,
                                       SetIntensityOperate>;

    struct ClearOperate {
        std::optional<SlotId> slot_id;
        std::optional<Channel> channel;
    };

    struct DeviceOperateResult {
        ActionType type{};
        TaskEndReason reason{};
        std::optional<SlotId> slot_id;
        std::optional<Channel> channel;
    };

    void to_json(nlohmann::json& j, const DeviceOperate& value);
    void from_json(const nlohmann::json& j, DeviceOperate& value);
    void to_json(nlohmann::json& j, const ClearOperate& value);
    void from_json(const nlohmann::json& j, ClearOperate& value);
    void to_json(nlohmann::json& j, const DeviceOperateResult& value);
    void from_json(const nlohmann::json& j, DeviceOperateResult& value);
}

#endif // ISAACCOYOTE_APP_COYOTE_WEBSOCKET_OPERATION_H
