//
// Created by TsCat on 2026/7/21.
//

#include "app/service/coyote/pulse_helper/pulse_helper.h"

#include <iterator>

#include "app/service/log/log_service.h"

using namespace app::coyote::pulse;

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.coyote");
        return *logger;
    }

    PulseTrack repeat_track(const PulseTrack& track, size_t frame_count) {
        if (track.empty() || frame_count <= 0) return {};

        PulseTrack result;
        result.reserve(frame_count);

        for (std::size_t i = 0; i < frame_count; ++i) {
            result.emplace_back(
                track[i % track.size()]);
        }

        return result;
    }

    PulseTrack repeat_track(const PulseTrack& track, std::chrono::milliseconds duration) {
        using namespace std::chrono_literals;
        if (track.empty() || duration <= std::chrono::milliseconds::zero()) return {};

        auto frame_count = static_cast<std::size_t>((duration + 99ms) / 100ms); //round up

        return repeat_track(track, frame_count);
    }
}

PulseHelper::PulseHelper(const config::ConfigService& config) : config_(config) {}

bool PulseHelper::set_loop(Channel channel, const std::string& name) {
    if (auto it = pulse_definitions.find(name); it != pulse_definitions.end()) {
        set_loop(channel, it->second);
        return true;
    }

    log_().warn("pulse definition not found: \"{}\"", name);
    return false;
}

void PulseHelper::set_loop(Channel channel, const PulseTrack& pulse_track) {
    auto& state = (channel == Channel::A) ? a_ : b_;

    state.loop = pulse_track;
    state.loop_index = 0;
}

bool PulseHelper::push_front(Channel channel, const std::string& name, std::chrono::milliseconds duration) {
    if (auto it = pulse_definitions.find(name); it != pulse_definitions.end()) {
        push_front(channel, it->second, duration);
        return true;
    }
    log_().warn("pulse definition not found: \"{}\"", name);
    return false;
}

void PulseHelper::push_front(Channel channel, const PulseTrack& pulse_track, std::chrono::milliseconds duration) {
    PulseTrack repeated_track = repeat_track(pulse_track, duration);
    auto& state = (channel == Channel::A) ? a_ : b_;

    state.pending.insert(state.pending.begin(),
                         std::make_move_iterator(repeated_track.begin()),
                         std::make_move_iterator(repeated_track.end()));
}

bool PulseHelper::push_back(Channel channel, const std::string& name, std::chrono::milliseconds duration) {
    if (auto it = pulse_definitions.find(name); it != pulse_definitions.end()) {
        push_back(channel, it->second, duration);
        return true;
    }
    log_().warn("pulse definition not found: \"{}\"", name);
    return false;
}

void PulseHelper::push_back(Channel channel, const PulseTrack& pulse_track, std::chrono::milliseconds duration) {
    PulseTrack repeated_track = repeat_track(pulse_track, duration);
    auto& state = (channel == Channel::A) ? a_ : b_;

    state.pending.insert(state.pending.end(),
                         std::make_move_iterator(repeated_track.begin()),
                         std::make_move_iterator(repeated_track.end()));
}

PulseTrack PulseHelper::pop(Channel channel, std::chrono::milliseconds duration) {
    using namespace std::chrono_literals;
    if (duration <= 0ms) return {};
    if (channel != Channel::A && channel != Channel::B) return {};

    auto frame_count = static_cast<std::size_t>((duration + 99ms) / 100ms); //round up

    auto& state = (channel == Channel::A) ? a_ : b_;

    PulseTrack result;
    auto pending_count = std::min(frame_count, state.pending.size());
    if (pending_count > 0) {
        auto end = std::next(
            state.pending.begin(), static_cast<std::ptrdiff_t>(pending_count)
        );
        result.append_range(std::ranges::subrange(state.pending.begin(), end));
        state.pending.erase(state.pending.begin(), end);
    }

    const auto missing_count = frame_count - pending_count;
    if (missing_count > 0 && !state.loop.empty()) {
        result.reserve(frame_count);
        for (std::size_t i = 0; i < missing_count; ++i) {
            result.push_back(state.loop[state.loop_index]);
            state.loop_index = (state.loop_index + 1) % state.loop.size();
        }
    }

    return result;
}

std::vector<std::string> PulseHelper::hex_pop(Channel channel, std::chrono::milliseconds duration) {
    PulseTrack track = pop(channel, duration);

    std::vector<std::string> hex_track;
    for (auto& frame : track) {
        hex_track.emplace_back(frame.to_hex());
    }

    return hex_track;
}

void PulseHelper::reload() {
    std::unordered_map<std::string, PulseTrack> def;
    for (const auto& [name, raw_pulse] : config_.snapshot()->pulse_definitions) {
        PulseTrack track;

        track.reserve(raw_pulse.size());
        for (const auto& raw_frame : raw_pulse) {
            track.push_back(PulseFrame::from_hex(raw_frame));
        }

        def.emplace(name, track);
    }
    pulse_definitions.swap(def);
}
