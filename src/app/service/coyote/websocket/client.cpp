//
// Created by TsCat on 2026/7/20.
//

#include "app/service/coyote/websocket/client.h"

#include <utility>

#include "app/service/log/log_service.h"

using namespace app::coyote::websocket;

namespace
{
    void apply_slots_patch(model::ComfortLimit& target, const model::ComfortLimitPatch& patch) {
        if (patch.mode) target.mode = *patch.mode;
        if (patch.comfort_max) target.comfort_max = *patch.comfort_max;
        if (patch.absolute_max) target.absolute_max = *patch.absolute_max;
        if (patch.overheat) target.overheat = *patch.overheat;
        if (patch.overheat_percent) target.overheat_percent = *patch.overheat_percent;
        if (patch.auto_increment) target.auto_increment = *patch.auto_increment;
        if (patch.auto_increment_max) target.auto_increment_max = *patch.auto_increment_max;
        if (patch.auto_increment_scope) target.auto_increment_scope = *patch.auto_increment_scope;
        if (patch.total_increment) target.total_increment = *patch.total_increment;
    }

    void apply_slots_patch(model::ChannelState& target, const model::ChannelStatePatch& patch) {
        if (patch.is_muted) target.is_muted = *patch.is_muted;
        if (patch.warm_up_scale) target.warm_up_scale = *patch.warm_up_scale;
        if (patch.intensity_max) target.intensity_max = *patch.intensity_max;
        if (patch.comfort_limit) apply_slots_patch(target.comfort_limit, *patch.comfort_limit);
    }

    void apply_patch(model::SlotState& target, const model::SlotStatePatch& patch) {
        if (patch.mark_light) target.mark_light = *patch.mark_light;
        if (patch.has_device) target.has_device = *patch.has_device;
        if (patch.channel_a) {
            if (!target.channel_a) target.channel_a.emplace();
            apply_slots_patch(*target.channel_a, *patch.channel_a);
        }
        if (patch.channel_b) {
            if (!target.channel_b) target.channel_b.emplace();
            apply_slots_patch(*target.channel_b, *patch.channel_b);
        }
    }

    void apply_patch(model::CoyoteProps& target, const model::CoyotePropsPatch& patch) {
        if (patch.power) target.power = *patch.power;
        if (patch.version) target.version = *patch.version;
        if (patch.label) target.label = *patch.label;
        if (patch.intensity_a) target.intensity_a = *patch.intensity_a;
        if (patch.intensity_b) target.intensity_b = *patch.intensity_b;
        if (patch.connect_state) target.connect_state = *patch.connect_state;
        if (patch.channel_a_status) target.channel_a_status = *patch.channel_a_status;
        if (patch.channel_b_status) target.channel_b_status = *patch.channel_b_status;
        if (patch.update_value) target.update_value = *patch.update_value;
    }

    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.coyote");
        return *logger;
    }
}


WsRemoteClient::WsRemoteClient(std::string client_id, WsController& controller) : controller_(controller),
    client_id_(std::move(client_id)) {}

void WsRemoteClient::broadcast_intensity(model::Channel channel, int intensity) {
    std::vector<model::SlotId> slot_ids;
    {
        std::lock_guard lock(device_mutex_);
        slot_ids.reserve(cached_devices_.size());
        for (const auto& [slot_id, _] : cached_devices_)
            slot_ids.push_back(slot_id);
    }
    for (const auto& slot_id : slot_ids) {
        set_intensity(slot_id, channel, intensity);
    }
}

void WsRemoteClient::broadcast_pulse(model::Channel channel, model::PulseTrack track) {
    std::vector<model::SlotId> slot_ids;
    {
        std::lock_guard lock(device_mutex_);
        slot_ids.reserve(cached_devices_.size());
        for (const auto& [slot_id, _] : cached_devices_)
            slot_ids.push_back(slot_id);
    }
    for (const auto& slot_id : slot_ids) {
        append_pulse(slot_id, channel, track, false);
    }
}

std::future<model::DeviceOperateResult> WsRemoteClient::append_pulse(
    model::SlotId slot_id, model::Channel channel,
    model::PulseTrack pulse, bool immediate, model::Priority priority
) {
    return operate(model::AppendPulseDataOperate{
        .common = model::OperateCommon{
            .slot_id = slot_id,
            .channel = channel,
            .priority = priority,
            .duration_ms = pulse.size() * 100,
            .immediate = immediate,
        },
        .value = pulse,
        .sequence = std::nullopt
    });
}

std::optional<int> WsRemoteClient::get_current_intensity(model::SlotId slot_id, model::Channel channel) const {
    std::lock_guard lock(device_mutex_);
    if (auto it = cached_devices_.find(slot_id); it != cached_devices_.end()) {
        const auto& props = it->second.props;
        if (props) {
            if (channel == model::Channel::A && props->channel_a_status)
                return props->intensity_a;
            if (channel == model::Channel::B && props->channel_b_status)
                return props->intensity_b;
        }
    }
    return std::nullopt;
}

std::optional<int> WsRemoteClient::get_max_intensity(model::SlotId slot_id, model::Channel channel) const {
    std::lock_guard lock(device_mutex_);
    if (auto it = cached_devices_.find(slot_id); it != cached_devices_.end()) {
        const auto& slot_state = it->second.slot_state;
        if (!slot_state.has_value()) return std::nullopt;

        if (channel == model::Channel::A) {
            const auto& ch_a = slot_state->channel_a;
            return ch_a.has_value() ? std::optional{ch_a->intensity_max} : std::nullopt;
        }
        if (channel == model::Channel::B) {
            const auto& ch_b = slot_state->channel_b;
            return ch_b.has_value() ? std::optional{ch_b->intensity_max} : std::nullopt;
        }
    }
    return std::nullopt;
}

std::future<model::DeviceOperateResult> WsRemoteClient::set_intensity(
    model::SlotId slot_id, model::Channel channel,
    int intensity, model::Priority priority
) {
    return operate(model::SetTempIntensityOperate{
        .common = model::OperateCommon{
            .slot_id = slot_id,
            .channel = channel,
            .priority = priority,
            .duration_ms = 0,
            .immediate = true
        },
        .value = intensity,
    });
}

std::future<model::DeviceOperateResult> WsRemoteClient::reset_intensity(model::SlotId slot_id, model::Channel channel) {
    return operate(model::SetIntensityOperate{
        .common = model::OperateCommon{
            .slot_id = slot_id,
            .channel = channel,
            .priority = model::Priority::Normal,
            .immediate = true,
        },
        .value = 0,
    });
}

std::future<model::DeviceOperateResult> WsRemoteClient::operate(model::DeviceOperate operation) {
    return request<model::DeviceOperateResult>(model::RpcMethod::DeviceOperate, operation);
}

std::future<void> WsRemoteClient::clear(model::ClearOperate operation) {
    return request<void>(model::RpcMethod::DeviceOperateClear, operation);
}

std::future<model::DevicesGetResult> WsRemoteClient::get_devices() {
    return request<model::DevicesGetResult>(model::RpcMethod::DevicesGet, {});
}

std::future<model::Timestamp> WsRemoteClient::ping() {
    return request<model::Timestamp>(model::RpcMethod::Ping, {});
}

const model::ClientId& WsRemoteClient::get_id() const {
    return client_id_;
}

int WsRemoteClient::alive_device_count() const {
    std::lock_guard lock(device_mutex_);

    int count = 0;
    for (auto [_, device]: cached_devices_) {
        if (device.props.has_value()) {
            if (device.props.value().connect_state=="connected") ++ count;
        }
    }
    return count;
}

void WsRemoteClient::clean_pending() {
    const auto now = std::chrono::steady_clock::now();

    std::vector<std::unique_ptr<PendingRequestBase>> timed_out_requests;
    {
        std::lock_guard lock(pending_mutex_);
        for (auto it = pending_.begin(); it != pending_.end();) {
            auto& req = it->second;
            if (req->no_timeout) {
                it = pending_.erase(it);
            }
            else if (now - req->created_at >= req->timeout + std::chrono::milliseconds(2000)) {
                timed_out_requests.emplace_back(std::move(req));
                it = pending_.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    for (auto& req : timed_out_requests) {
        req->fail(std::make_exception_ptr(std::runtime_error("Request timed out")));
    }
}

void WsRemoteClient::set_event_callback(model::EventType event_type, EventCallback callback) {
    event_callbacks_.insert_or_assign(event_type, callback);
}

void WsRemoteClient::cancel_all_pending() {
    std::lock_guard lock(pending_mutex_);
    for (auto& [id, request] : pending_)
        request->fail(std::make_exception_ptr(std::runtime_error("WebSocket stopping")));
    pending_.clear();
}

void WsRemoteClient::handle_message(const model::AppMessage& message) {
    std::visit([this]<typename T>(const T& m)
    {
        if constexpr (std::is_same_v<T, model::RpcResponse>) handle_response(m);
        else if constexpr (std::is_same_v<T, model::AppEvent>) handle_event(m);
    }, message);
}

void WsRemoteClient::handle_response(const model::RpcResponse& response) {
    std::unique_ptr<PendingRequestBase> pending;

    {
        std::lock_guard lock(pending_mutex_);
        if (auto it = pending_.find(response.request_id); it != pending_.end()) {
            pending = std::move(it->second);
            pending_.erase(it);
        }
    }

    if (!pending)
        return;

    if (response.error.has_value()) {
        log_().warn("rpc {} failed: {}", response.request_id, response.error.value());
        pending->fail(std::make_exception_ptr(std::runtime_error(response.error.value())));
        return;
    }

    if (!response.result) {
        log_().warn("rpc {} returned no result", response.request_id);
        pending->fail(std::make_exception_ptr(std::runtime_error("RPC response has no result")));
        return;
    }

    try {
        pending->complete(*response.result);
    }
    catch (...) {
        pending->fail(std::current_exception());
    }
}

void WsRemoteClient::handle_event(const model::AppEvent& event) {
    if (event.event == model::EventType::DevicesSnapshot) {
        if (auto* snapshot = std::get_if<model::DevicesSnapshotEvent>(&event.data))
            handle_devices_snapshot(*snapshot);
    }

    if (event.event == model::EventType::DevicesPatch) {
        if (auto* patch = std::get_if<model::DevicesPatchEvent>(&event.data))
            handle_devices_patch(*patch);
    }

    if (event.event == model::EventType::SlotsPatch) {
        if (auto* patch = std::get_if<model::SlotsPatchEvent>(&event.data))
            handle_slots_patch(*patch);
    }

    if (auto it = event_callbacks_.find(event.event); it != event_callbacks_.end()) {
        it->second(event);
    }
}

void WsRemoteClient::handle_devices_snapshot(const model::DevicesSnapshotEvent& event) {
    std::unordered_map<model::SlotId, model::RemoteDevice> devices;
    for (const auto& device : event.devices)
        devices.emplace(device.slot_id, device);

    {
        std::lock_guard lock(device_mutex_);
        cached_devices_.swap(devices);
    }

    for (const auto& device : event.devices) {
        reset_intensity(device.slot_id, model::Channel::A);
        reset_intensity(device.slot_id, model::Channel::B);
    }
}

void WsRemoteClient::handle_devices_patch(const model::DevicesPatchEvent& event) {
    std::vector<model::SlotId> added_slot_ids;
    {
        std::lock_guard lock(device_mutex_);

        for (const auto& device : event.added) {
            cached_devices_.insert_or_assign(device.slot_id, device);
            added_slot_ids.push_back(device.slot_id);
        }
        for (const auto& slot_id : event.removed)
            cached_devices_.erase(slot_id);
    }

    for (const auto& slot_id : added_slot_ids) {
        reset_intensity(slot_id, model::Channel::A);
        reset_intensity(slot_id, model::Channel::B);
    }
}

void WsRemoteClient::handle_slots_patch(const model::SlotsPatchEvent& event) {
    std::lock_guard lock(device_mutex_);

    auto try_apply_patch = [](auto& target, const auto& patch_val)
    {
        if (patch_val && target) {
            apply_patch(*target, *patch_val);
        }
        // ignore if patch has no value
    };

    for (const auto& patch : event.slots) {
        if (auto it = cached_devices_.find(patch.slot_id); it != cached_devices_.end()) {
            try_apply_patch(it->second.slot_state, patch.slot_state);
            try_apply_patch(it->second.props, patch.props);
        }
    }
}


model::RequestId WsRemoteClient::next_request_id() {
    return std::to_string(request_id_.fetch_add(1, std::memory_order_relaxed) + 1);
}
