//
// Created by TsCat on 2026/7/29.
//

#include "app/service/overlay/overlay_ui.h"

#include <imgui.h>

#include "app/service/overlay/coyote_monitor.h"
#include "app/service/overlay/tab_manager.h"

using namespace app::overlay;

OverlayUi::OverlayUi(game::Game& game, config::ConfigService& config) : config_(config) {
    tab_manager_ = std::make_unique<TabManager>(input_state_, game, config_);
    monitor_ = std::make_unique<CoyoteMonitor>(game);
}

OverlayUi::~OverlayUi() = default;

void OverlayUi::handle_key_up(const int key) {
    input_state_.handle_key_up(key, config_.snapshot()->overlay.menu_key);
}

void OverlayUi::render() {
    const bool menu_open = input_state_.is_menu_open();
    ImGui::GetIO().MouseDrawCursor = menu_open;

    if (menu_open) tab_manager_->render();
    monitor_->render(menu_open);
}
