// Created by TsCat on 2026/8/19.
#include <imgui.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"

using namespace app::config;

// 日志页:级别与输出。

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;
    }

    void ConfigTab::draw_logging() {
        if (ui::begin_inset_panel("logging_card")) {
            ui::section_header("日志", "级别与输出");

            if (ui::begin_property_table("logging_table")) {
                static constexpr const char* kLevels[] = {"trace", "debug", "info", "warn", "error", "critical", "off"};
                int index = 0;
                for (int i = 0; i < IM_ARRAYSIZE(kLevels); ++i) {
                    if (state_.draft.logging.level == kLevels[i]) {
                        index = i;
                        break;
                    }
                }
                ui::property_row("全局级别");
                if (ui::combo("log_level", &index, kLevels, IM_ARRAYSIZE(kLevels))) {
                    state_.draft.logging.level = kLevels[index];
                    state_.dirty = true;
                }
                ImGui::EndTable();
            }
            ui::end_inset_panel();
        }
    }
}