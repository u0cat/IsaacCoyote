//
// Created by TsCat on 2026/7/21.
//

#ifndef ISAACCOYOTE_PULSE_HELPER_H
#define ISAACCOYOTE_PULSE_HELPER_H
#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "app/service/config/config_service.h"
#include "types.h"

namespace app::coyote::pulse
{
    class PulseHelper {
    public:
        PulseHelper(const config::ConfigService& config);

        bool set_loop(Channel channel, const std::string& name);
        void set_loop(Channel channel, const PulseTrack& pulse_track);

        bool push_front(Channel channel, const std::string& name, std::chrono::milliseconds duration);
        void push_front(Channel channel, const PulseTrack& pulse_track, std::chrono::milliseconds duration);

        bool push_back(Channel channel, const std::string& name, std::chrono::milliseconds duration);
        void push_back(Channel channel, const PulseTrack& pulse_track, std::chrono::milliseconds duration);

        PulseTrack pop(Channel channel, std::chrono::milliseconds duration);
        std::vector<std::string> hex_pop(Channel channel, std::chrono::milliseconds duration);
        void reload();

    private:
        struct ChannelState {
            PulseTrack loop{};
            std::size_t loop_index = 0;
            std::deque<PulseFrame> pending;
        };

        ChannelState a_;

        ChannelState b_;

        std::unordered_map<std::string, PulseTrack> pulse_definitions;
        const config::ConfigService& config_;
    };
}

#endif //ISAACCOYOTE_PULSE_HELPER_H
