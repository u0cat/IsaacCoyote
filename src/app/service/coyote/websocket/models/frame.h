#ifndef ISAACCOYOTE_APP_COYOTE_WEBSOCKET_FRAME_H
#define ISAACCOYOTE_APP_COYOTE_WEBSOCKET_FRAME_H

#include <optional>
#include <string>
#include <variant>

#include <nlohmann/json.hpp>

#include "message.h"

namespace app::coyote::websocket::model
{
    struct HelloFrame { ClientId client_id; };
    struct ClientAttachedFrame { ClientId client_id; };
    struct ClientDisconnectedFrame { ClientId client_id; };
    struct ControllerAttachedFrame { ClientId client_id; };
    struct ControllerDisconnectedFrame { ClientId client_id; };
    struct HeartbeatFrame {};
    struct PingFrame {};

    struct PongFrame
    {
        std::optional<Timestamp> timestamp;
    };

    struct MessageFrame
    {
        ClientId client_id;
        AppMessage data;
    };

    struct IdleTimeoutFrame {};

    struct ErrorFrame
    {
        std::string error;
        nlohmann::json data = nlohmann::json::object();
    };

    using ServerFrame = std::variant<HelloFrame,
                                     ClientAttachedFrame,
                                     ClientDisconnectedFrame,
                                     ControllerAttachedFrame,
                                     ControllerDisconnectedFrame,
                                     HeartbeatFrame,
                                     PingFrame,
                                     PongFrame,
                                     MessageFrame,
                                     IdleTimeoutFrame,
                                     ErrorFrame>;

    struct CloseInfo
    {
        std::uint16_t code{};
        std::string reason;
    };

    inline constexpr std::uint16_t kControllerDisconnectedCloseCode = 4000;
    inline constexpr std::uint16_t kControllerNotFoundCloseCode = 4001;
    inline constexpr std::uint16_t kIdleTimeoutCloseCode = 4002;

    void to_json(nlohmann::json& j, const ServerFrame& value);
    void from_json(const nlohmann::json& j, ServerFrame& value);
    void to_json(nlohmann::json& j, const CloseInfo& value);
    void from_json(const nlohmann::json& j, CloseInfo& value);
}

#endif // ISAACCOYOTE_APP_COYOTE_WEBSOCKET_FRAME_H
