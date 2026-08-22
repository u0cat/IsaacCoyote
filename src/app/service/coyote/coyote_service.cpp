//
// Created by TsCat on 2026/7/19.
//

#include "app/service/coyote/coyote_service.h"

#include <algorithm>

using namespace app::coyote;

namespace
{
    constexpr std::chrono::milliseconds kTickInterval{150};
    constexpr std::chrono::milliseconds kPulseInterval{200};
    constexpr std::chrono::seconds kMonitorHistory{2};
    constexpr std::chrono::milliseconds kPulseFrameDuration{100};
}

CoyoteService::CoyoteService(const config::ConfigService& config) : config_(config) {
    ws_controller_ = std::make_unique<websocket::WsController>(config.snapshot()->ws_endpoint);
    pulse_helper_ = std::make_unique<pulse::PulseHelper>(config_);
    pulse_helper_->reload();
}

void CoyoteService::start() {
    ws_controller_->start();
}

bool CoyoteService::is_ready() const {
    return ws_controller_->is_ready();
}

void CoyoteService::stop() {
    ws_controller_->stop();
}

websocket::WsController* CoyoteService::get_ws_controller() {
    return ws_controller_.get();
}

pulse::PulseHelper* CoyoteService::get_pulse_helper() {
    return pulse_helper_.get();
}

CoyoteService::MonitorSnapshot CoyoteService::monitor_snapshot() const {
    return monitor_snapshot_;
}

void CoyoteService::reload_pulses() {
    pulse_helper_->reload();
}

void CoyoteService::set_loop(const std::string& name_a, const std::string& name_b) {
    if (name_a.empty())
        pulse_helper_->set_loop(pulse::Channel::A, pulse::PulseTrack{});
    else
        pulse_helper_->set_loop(pulse::Channel::A, name_a);

    if (name_b.empty())
        pulse_helper_->set_loop(pulse::Channel::B, pulse::PulseTrack{});
    else
        pulse_helper_->set_loop(pulse::Channel::B, name_b);
}

void CoyoteService::push_pulse(const std::string& name_a, const std::string& name_b, std::chrono::milliseconds duration) {
    if (!name_a.empty())
        pulse_helper_->push_front(pulse::Channel::A, name_a, duration);
    if (!name_b.empty())
        pulse_helper_->push_front(pulse::Channel::B, name_b, duration);
}

void CoyoteService::tick(std::chrono::steady_clock::time_point now, int intensity_a, int intensity_b) {
    if (last_tick_ != std::chrono::steady_clock::time_point{} && now - last_tick_ < kTickInterval) return;
    last_tick_ = now;

    ws_controller_->clean();

    ws_controller_->broadcast_intensity(websocket::model::Channel::A, intensity_a);
    ws_controller_->broadcast_intensity(websocket::model::Channel::B, intensity_b);

    const auto pulse_a = pulse_helper_->pop(pulse::Channel::A, kPulseInterval);
    const auto pulse_b = pulse_helper_->pop(pulse::Channel::B, kPulseInterval);
    publish_monitor_samples(now, pulse_a, pulse_b);

    const auto to_hex_track = [](const pulse::PulseTrack& track) {
        std::vector<std::string> result;
        result.reserve(track.size());
        for (const auto& frame : track)
            result.push_back(frame.to_hex());
        return result;
    };

    if (!pulse_a.empty())
        ws_controller_->broadcast_pulse(websocket::model::Channel::A, to_hex_track(pulse_a));
    if (!pulse_b.empty())
        ws_controller_->broadcast_pulse(websocket::model::Channel::B, to_hex_track(pulse_b));
}

void CoyoteService::publish_monitor_samples(const std::chrono::steady_clock::time_point now,
                                            const pulse::PulseTrack& pulse_a,
                                            const pulse::PulseTrack& pulse_b) {
    auto next = std::make_shared<PulseMonitorSnapshot>();
    *next = *monitor_snapshot_;

    const auto append = [now](std::vector<PulseMonitorSample>& samples, const pulse::PulseTrack& track) {
        auto sample_time = now;
        const auto sample_count = track.size() * pulse::PulseFrame{}.strength.size();
        const auto sample_interval = sample_count > 0
                                         ? kTickInterval / static_cast<int>(sample_count)
                                         : std::chrono::milliseconds::zero();

        for (const auto& frame : track) {
            for (const auto strength : frame.strength) {
                samples.push_back({sample_time, strength});
                sample_time += sample_interval;
            }
        }

        const auto oldest = now - kMonitorHistory;
        std::erase_if(samples, [oldest](const PulseMonitorSample& sample) {
            return sample.created_at < oldest;
        });
    };

    append(next->channel_a, pulse_a);
    append(next->channel_b, pulse_b);

    monitor_snapshot_ = std::move(next);
}
