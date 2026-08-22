//
// Created by TsCat on 2026/7/19.
//

#ifndef ISAACCOYOTE_COYOTE_SERVICE_H
#define ISAACCOYOTE_COYOTE_SERVICE_H
#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include "pulse_helper/pulse_helper.h"
#include "websocket/controller.h"

namespace app::config
{
    class ConfigService;
}

namespace app::coyote
{
    struct PulseMonitorSample {
        std::chrono::steady_clock::time_point created_at;
        std::uint8_t strength{};
    };

    struct PulseMonitorSnapshot {
        std::vector<PulseMonitorSample> channel_a;
        std::vector<PulseMonitorSample> channel_b;
    };

    class CoyoteService {
    public:
        using MonitorSnapshot = std::shared_ptr<const PulseMonitorSnapshot>;

        CoyoteService(const config::ConfigService& config);

        void set_loop(const std::string& name_a, const std::string& name_b);
        void push_pulse(const std::string& name_a, const std::string& name_b, std::chrono::milliseconds duration);

        void reload_pulses();
        void tick(std::chrono::steady_clock::time_point now, int intensity_a, int intensity_b);

        void start();
        bool is_ready() const;
        void stop();

        websocket::WsController* get_ws_controller();
        pulse::PulseHelper* get_pulse_helper();
        [[nodiscard]] MonitorSnapshot monitor_snapshot() const;

    private:
        void publish_monitor_samples(std::chrono::steady_clock::time_point now,
                                     const pulse::PulseTrack& pulse_a,
                                     const pulse::PulseTrack& pulse_b);

        std::unique_ptr<websocket::WsController> ws_controller_;
        std::unique_ptr<pulse::PulseHelper> pulse_helper_;

        std::chrono::steady_clock::time_point last_tick_{};
        const config::ConfigService& config_;

        MonitorSnapshot monitor_snapshot_ = std::make_shared<const PulseMonitorSnapshot>();
    };
}

#endif //ISAACCOYOTE_COYOTE_SERVICE_H
