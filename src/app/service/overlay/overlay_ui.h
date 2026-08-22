//
// Created by TsCat on 2026/7/29.
//

#ifndef ISAACCOYOTE_OVERLAY_UI_H
#define ISAACCOYOTE_OVERLAY_UI_H

#include <memory>

#include "app/game.h"
#include "app/service/config/config_service.h"
#include "app/service/overlay/core/input_state.h"

namespace app::overlay
{
    class CoyoteMonitor;
    class TabManager;

    class OverlayUi {
    public:
        OverlayUi(game::Game& game, config::ConfigService& config);
        ~OverlayUi();

        void handle_key_up(int key);
        void render();

    private:
        config::ConfigService& config_;
        InputState input_state_;
        std::unique_ptr<TabManager> tab_manager_;
        std::unique_ptr<CoyoteMonitor> monitor_;
    };
}

#endif // ISAACCOYOTE_OVERLAY_UI_H
