#ifndef ISAACCOYOTE_STATISTICS_TAB_H
#define ISAACCOYOTE_STATISTICS_TAB_H

#include <string>

#include "app/game.h"
#include "app/service/overlay/types.h"

namespace app::overlay::tabs
{
    class StatisticsTab final : public ITab {
    public:
        explicit StatisticsTab(game::Game& game) : game_(game) {};

        void render() override;
        std::string get_id() override { return "StatisticsTab"; }
        std::string get_display_name() override { return "统计"; }

    private:
        game::Game& game_;
    };
}

#endif
