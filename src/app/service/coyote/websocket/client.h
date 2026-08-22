//
// Created by TsCat on 2026/7/20.
//

#ifndef ISAACCOYOTE_CLIENT_H
#define ISAACCOYOTE_CLIENT_H
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "controller.h"

namespace app::coyote::websocket
{
    class WsRemoteClient {
    public:
        WsRemoteClient(model::ClientId client_id, WsController& controller);

        void broadcast_intensity(model::Channel channel, int intensity);
        void broadcast_pulse(model::Channel channel, model::PulseTrack track);

        std::future<model::DeviceOperateResult> append_pulse(
            model::SlotId slot_id, model::Channel channel,
            model::PulseTrack pulse, bool immediate = false, model::Priority priority = model::Priority::Normal
        );

        std::optional<int> get_current_intensity(model::SlotId slot_id, model::Channel channel) const;
        std::optional<int> get_max_intensity(model::SlotId slot_id, model::Channel channel) const;

        std::future<model::DeviceOperateResult> reset_intensity(model::SlotId slot_id, model::Channel channel);
        std::future<model::DeviceOperateResult> set_intensity(
            model::SlotId slot_id, model::Channel channel,
            int intensity, model::Priority priority = model::Priority::Normal
        );

        std::future<model::Timestamp> ping();
        std::future<model::DevicesGetResult> get_devices();
        std::future<void> clear(model::ClearOperate operation);
        std::future<model::DeviceOperateResult> operate(model::DeviceOperate operation);

        const model::ClientId& get_id() const;
        int alive_device_count() const;

        void cancel_all_pending();
        void clean_pending();

        using EventCallback = std::function<void(const model::AppEvent&)>;
        void set_event_callback(model::EventType event_type, EventCallback callback);

    private:
        struct PendingRequestBase {
            std::chrono::steady_clock::time_point created_at = std::chrono::steady_clock::now();
            std::chrono::milliseconds timeout = std::chrono::seconds{5};
            bool no_timeout = false;

            virtual ~PendingRequestBase() = default;
            virtual void complete(const model::RpcResult& result) = 0;
            virtual void fail(std::exception_ptr error) = 0;
        };

        template <typename result_t>
        struct PendingRequest;
        template <typename result_t>
        std::future<result_t> request(model::RpcMethod method, model::RpcRequestData data);

        void handle_message(const model::AppMessage& msg);
        void handle_response(const model::RpcResponse& msg);
        void handle_event(const model::AppEvent& msg);

        void handle_devices_snapshot(const model::DevicesSnapshotEvent& event);
        void handle_devices_patch(const model::DevicesPatchEvent& event);
        void handle_slots_patch(const model::SlotsPatchEvent& event);

        mutable std::mutex device_mutex_;
        std::unordered_map<model::SlotId, model::RemoteDevice> cached_devices_;

        mutable std::mutex pending_mutex_;
        std::unordered_map<model::RequestId, std::unique_ptr<PendingRequestBase>> pending_;
        std::unordered_map<model::EventType, EventCallback> event_callbacks_;

        const model::ClientId client_id_;
        WsController& controller_;

        std::atomic_uint64_t request_id_ = 0;
        model::RequestId next_request_id();

        // allow WsController route message to client
        friend class WsController;
    };

    template <typename result_t>
    struct WsRemoteClient::PendingRequest : PendingRequestBase {
        std::promise<result_t> promise;

        void complete(const model::RpcResult& result) override {
            try {
                promise.set_value(std::get<result_t>(result));
            }
            catch (const std::future_error&) {
            }
        }

        void fail(std::exception_ptr error) override {
            try {
                promise.set_exception(error);
            }
            catch (const std::future_error&) {
            }
        }
    };

    // void
    template <>
    struct WsRemoteClient::PendingRequest<void> : PendingRequestBase {
        std::promise<void> promise;

        void complete(const model::RpcResult& result) override {
            try {
                promise.set_value();
            }
            catch (const std::future_error&) {
            }
        }

        void fail(std::exception_ptr error) override {
            try {
                promise.set_exception(error);
            }
            catch (const std::future_error&) {
            }
        }
    };

    template <typename result_t>
    std::future<result_t> WsRemoteClient::request(model::RpcMethod method, model::RpcRequestData data) {
        auto pending = std::make_unique<PendingRequest<result_t>>();

        if (auto* operate = std::get_if<model::DeviceOperate>(&data)) {
            model::OperateCommon& common = std::visit(
                [](auto& operation) -> model::OperateCommon& {
                    return operation.common;
                },
                *operate
            );

            if (!common.duration_ms.has_value() || common.duration_ms.value() == 0) {
                pending->no_timeout = true;
            } else {
                pending->no_timeout = false;
                pending->timeout = std::chrono::milliseconds{common.duration_ms.value()};
            }
        }

        auto future = pending->promise.get_future();
        const model::RequestId request_id = next_request_id();

        {
            std::lock_guard lock(pending_mutex_);
            pending_.emplace(request_id, std::move(pending));
        }

        model::RpcRequest request{request_id, method, data};

        std::unique_ptr<PendingRequestBase> failed_request;
        if (!controller_.send_to(client_id_, request)) {
            {
                std::lock_guard lock(pending_mutex_);
                if (auto it = pending_.find(request_id); it != pending_.end()) {
                    failed_request = std::move(it->second);
                    pending_.erase(it);
                }
            }
            if (failed_request) failed_request->fail(std::make_exception_ptr(std::runtime_error("Failed to send")));
        }
        return future;
    }
}
#endif //ISAACCOYOTE_CLIENT_H
