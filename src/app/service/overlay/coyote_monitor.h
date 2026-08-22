//
// Created by TsCat on 2026/7/7.
//

#ifndef ISAACCOYOTE_MONITOR_H
#define ISAACCOYOTE_MONITOR_H
#include <chrono>
#include <vector>

#include <imgui.h>

#include "app/game.h"

namespace app::overlay
{
    class CoyoteMonitor {
    public:
        explicit CoyoteMonitor(game::Game& game) : game_(game) {}
        void render(bool interactive);

    private:
        game::Game& game_;

        static void draw_channel(
            const char* id,
            const std::vector<coyote::PulseMonitorSample>& samples,
            int current_value,
            int limit,
            ImVec2 size,
            std::chrono::steady_clock::time_point now
        );
    };
}

#endif //ISAACCOYOTE_MONITOR_H
