//
// Created by TsCat on 2026/7/7.
//

#include "app/service/overlay/tab_manager.h"

#include <imgui.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/connection_tab.h"
#include "app/service/overlay/tabs/debug_tab.h"
#include "app/service/overlay/tabs/statistics_tab.h"
#include "app/service/overlay/ui/themes.h"

using namespace app::overlay;

TabManager::TabManager(InputState& input_state, game::Game& game, config::ConfigService& config) {
    tabs_.push_back(std::make_unique<tabs::ConnectionTab>());
    tabs_.push_back(std::make_unique<tabs::ConfigTab>("Config", "配置", input_state, config));
    tabs_.push_back(std::make_unique<tabs::StatisticsTab>(game));
    tabs_.push_back(std::make_unique<tabs::DebugTab>("Debug", "调试"));
}

void TabManager::render() {
    ImGui::SetNextWindowSize(ui::metrics::kDefaultWindowSize, ImGuiCond_FirstUseEver);

    if (ImGui::Begin("IsaacCoyote")) {
        if (ImGui::BeginTabBar("MainTabBar")) {
            for (const auto& tab : tabs_) {
                if (ImGui::BeginTabItem(tab->get_display_name().c_str())) {
                    tab->render();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
