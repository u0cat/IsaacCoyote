//
// Created by TsCat on 2026/7/9.
//

#ifndef ISAACCOYOTE_DEBUG_TAB_H
#define ISAACCOYOTE_DEBUG_TAB_H
#include <string>

#include "app/service/overlay/types.h"
#include "isaac_spy/isaac/game.h"

namespace app::overlay::tabs
{
    class DebugTab : public ITab {
    public:
        DebugTab(std::string id, std::string display_name);
        std::string get_id() override { return id; };
        std::string get_display_name() override { return display_name; };
        void render() override;
        std::string display_name;

    private:
        std::string id;

        std::string collectible_query;
        int query_quality = -1;
    };
}
#endif //ISAACCOYOTE_DEBUG_TAB_H
