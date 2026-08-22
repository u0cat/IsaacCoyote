//
// Created by TsCat on 2026/7/7.
//

#ifndef ISAACCOYOTE_OVERLAY_TAB_MANAGER_H
#define ISAACCOYOTE_OVERLAY_TAB_MANAGER_H
#include <memory>
#include <vector>

#include "app/service/overlay/core/input_state.h"
#include "app/service/overlay/types.h"

namespace app::game
{
    class Game;
}

namespace app::config
{
    class ConfigService;
}

namespace app::overlay
{
    class TabManager {
    public:
        TabManager(InputState& input_state, game::Game& game, config::ConfigService& config);

        void render();

    private:
        std::vector<std::unique_ptr<ITab>> tabs_;
    };
}
#endif //ISAACCOYOTE_OVERLAY_TAB_MANAGER_H
