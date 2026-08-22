//
// Created by TsCat on 2026/7/20.
//

#include "app/service/coyote/websocket/controller.h"

#include <exception>
#include <format>

#include <ixwebsocket/IXNetSystem.h>

#include "app/service/coyote/websocket/client.h"
#include "app/service/log/log_service.h"

using namespace app::coyote::websocket;

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.coyote");
        return *logger;
    }
}

WsController::WsController(std::string_view endpoint) {
    ix::initNetSystem();

    websocket_.setUrl(endpoint.data());
    websocket_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Message) {
            try {
                handle_frame(nlohmann::json::parse(msg->str).get<model::ServerFrame>());
            }
            catch (const std::exception& e) {
                log_().error("dropped frame: {}", e.what());
            }
            catch (...) {
                log_().error("dropped frame: unknown exception");
            }
        }
        if (msg->type == ix::WebSocketMessageType::Open) {
            log_().info("websocket connected: {}", websocket_.getUrl());
        }
        if (msg->type == ix::WebSocketMessageType::Close) {
            log_().info("websocket closed: code={} reason={}",
                        msg->closeInfo.code, msg->closeInfo.reason);
        }
        if (msg->type == ix::WebSocketMessageType::Error) {
            log_().error("websocket error: {}", msg->errorInfo.reason);
        }
        if (msg->type == ix::WebSocketMessageType::Close || msg->type == ix::WebSocketMessageType::Error) {
            stopping_.store(true, std::memory_order_release);
        }
    });
}

WsController::~WsController() {
    websocket_.stop();
    ix::uninitNetSystem();
}

void WsController::start() {
    if (initialized_.load(std::memory_order_acquire)) {
        if (!stopping_.load(std::memory_order_acquire))
            return;

        stop();
    }

    stopping_.store(false, std::memory_order_release);
    initialized_.store(true, std::memory_order_release);
    log_().info("websocket connecting: {}", websocket_.getUrl());
    websocket_.start();
}

void WsController::stop() {
    stopping_.store(true, std::memory_order_release);
    websocket_.stop();
    log_().info("websocket stopped");

    initialized_.store(false, std::memory_order_release);

    {
        std::lock_guard lock(clients_mutex_);
        for (auto& [id, client] : clients_)
            client->cancel_all_pending();
        clients_.clear();
    }
    {
        std::lock_guard lock(state_mutex_);
        client_id_.clear();
    }
}

bool WsController::is_ready() {
    std::lock_guard lock(state_mutex_);
    return !stopping_.load() && !client_id_.empty() && websocket_.getReadyState() == ix::ReadyState::Open;
}

std::vector<model::ClientId> WsController::client_ids() const {
    std::lock_guard lock(clients_mutex_);

    std::vector<model::ClientId> ids;

    ids.reserve(clients_.size());
    for (auto&& [id, _] : clients_)
        ids.emplace_back(id);

    return ids;
}

std::shared_ptr<WsRemoteClient> WsController::find_client(const model::ClientId& client_id) const {
    std::lock_guard lock(clients_mutex_);

    if (auto it = clients_.find(client_id); it != clients_.end()) {
        return it->second;
    }

    return nullptr;
}

void WsController::broadcast_intensity(model::Channel channel, int intensity) {
    std::vector<std::shared_ptr<WsRemoteClient>> clients;
    {
        std::lock_guard lock(clients_mutex_);
        clients.reserve(clients_.size());
        for (const auto& [id, client] : clients_)
            clients.push_back(client);
    }
    for (const auto& client : clients) {
        client->broadcast_intensity(channel, intensity);
    }
}

void WsController::broadcast_pulse(model::Channel channel, model::PulseTrack track) {
    std::vector<std::shared_ptr<WsRemoteClient>> clients;
    {
        std::lock_guard lock(clients_mutex_);
        clients.reserve(clients_.size());
        for (const auto& [id, client] : clients_)
            clients.push_back(client);
    }
    for (const auto& client : clients) {
        client->broadcast_pulse(channel, track);
    }
}

void WsController::clean() {
    std::vector<std::shared_ptr<WsRemoteClient>> clients;
    {
        std::lock_guard lock(clients_mutex_);
        clients.reserve(clients_.size());
        for (const auto& [id, client] : clients_)
            clients.push_back(client);
    }
    for (const auto& client : clients) {
        client->clean_pending();
    }
}

std::string WsController::get_address() {
    std::lock_guard lock(state_mutex_);
    return std::format("{}?tid={}", websocket_.getUrl(), client_id_);
}

bool WsController::send_to(const model::ClientId& client_id, const model::AppMessage& msg) {
    if (!is_ready())
        return false;

    websocket_.send(
        nlohmann::json(model::MessageFrame{client_id, msg}).dump()
    );
    return true;
}

void WsController::handle_frame(const model::ServerFrame& frame) {
    std::visit([this]<typename T>(const T& f)
    {
        if constexpr (std::is_same_v<T, model::HelloFrame>) {
            std::lock_guard lock(state_mutex_);
            client_id_ = f.client_id;
            log_().info("server hello: tid={}", f.client_id);
        }
        else if constexpr (std::is_same_v<T, model::ClientAttachedFrame>) {
            std::lock_guard lock(clients_mutex_);
            clients_.try_emplace(f.client_id, std::make_unique<WsRemoteClient>(f.client_id, *this));
            log_().info("device attached: tid={}", f.client_id);
        }
        else if constexpr (std::is_same_v<T, model::ClientDisconnectedFrame>) {
            std::lock_guard lock(clients_mutex_);
            clients_.erase(f.client_id);
            log_().info("device detached: tid={}", f.client_id);
        }
        else if constexpr (std::is_same_v<T, model::IdleTimeoutFrame>) {
            log_().info("idle timeout, reconnecting");
            stop();
        }
        else if constexpr (std::is_same_v<T, model::ErrorFrame>) {
            log_().error("server error frame: {}", f.error);
            stop();
        }
        else if constexpr (std::is_same_v<T, model::MessageFrame>) {
            std::shared_ptr<WsRemoteClient> client;
            {
                std::lock_guard lock(clients_mutex_);
                if (auto it = clients_.find(f.client_id); it != clients_.end())
                    client = it->second;
            }
            if (client)
                client->handle_message(f.data);
            else
                log_().debug("message for unknown client: {}", f.client_id);
        }
    }, frame);
}

void WsController::remove_all_clients() {
    std::lock_guard lock(clients_mutex_);
    clients_.clear();
}
