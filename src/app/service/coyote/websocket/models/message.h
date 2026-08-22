#ifndef ISAACCOYOTE_APP_COYOTE_WEBSOCKET_MESSAGE_H
#define ISAACCOYOTE_APP_COYOTE_WEBSOCKET_MESSAGE_H

#include <optional>
#include <string>
#include <variant>

#include "device.h"
#include "operation.h"

namespace app::coyote::websocket::model
{
    struct EmptyData {};
    struct EmptyResult {};

    using EventData = std::variant<DevicesSnapshotEvent,
                                   DevicesPatchEvent,
                                   SlotsPatchEvent,
                                   CustomActionEvent>;

    struct AppEvent
    {
        EventType event{};
        EventData data;
    };

    using RpcRequestData = std::variant<EmptyData, DeviceOperate, ClearOperate>;

    struct RpcRequest
    {
        RequestId request_id;
        RpcMethod method{};
        RpcRequestData data;
    };

    using RpcResult = std::variant<EmptyResult, DevicesGetResult, Timestamp, DeviceOperateResult>;

    struct RpcResponse
    {
        RequestId request_id;
        std::optional<RpcResult> result;
        std::optional<std::string> error;
    };

    using AppMessage = std::variant<RpcRequest, RpcResponse, AppEvent>;

    void to_json(nlohmann::json& j, const AppEvent& value);
    void from_json(const nlohmann::json& j, AppEvent& value);
    void to_json(nlohmann::json& j, const RpcRequest& value);
    void from_json(const nlohmann::json& j, RpcRequest& value);
    void to_json(nlohmann::json& j, const RpcResponse& value);
    void from_json(const nlohmann::json& j, RpcResponse& value);
    void to_json(nlohmann::json& j, const AppMessage& value);
    void from_json(const nlohmann::json& j, AppMessage& value);
}

#endif // ISAACCOYOTE_APP_COYOTE_WEBSOCKET_MESSAGE_H
