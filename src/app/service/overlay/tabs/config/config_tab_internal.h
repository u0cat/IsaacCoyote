// Created by TsCat on 2026/8/19.
#ifndef ISAACCOYOTE_CONFIG_TAB_INTERNAL_H
#define ISAACCOYOTE_CONFIG_TAB_INTERNAL_H

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <imgui.h>

#include "app/service/overlay/tabs/config/config_tab.h"

// ConfigTab 各页翻译单元共享的内部辅助:定义集中在 config_tab.cpp(壳)。
namespace app::overlay::tabs::detail
{
    namespace layout
    {
        float kSidebarFooterHeight();
        float kPulseListWidth();
        float kPulseListCompactHeight();
        float kPulseEditorCompactHeight();
        float kPulseTextCompactHeight();
        float kPulseTextHeight();
        float kRuleListWidth();
        float kRuleListCompactHeight();
        float kPlayerIdsHeight();
    }

    extern const char* const kModifiers[4];
    extern const char* const kScopes[4];

    std::string trim_line(std::string value);
    bool valid_pulse_frame(const std::string& frame);

    bool draw_pulse_combo(const char* id,
                          const std::unordered_map<std::string, std::vector<std::string>>& pulses,
                          std::string& value);

    template <typename T>
    bool input(const char* id, T* value)
    {
        ImGui::SetNextItemWidth(-1.0f);
        const std::string hidden = app::overlay::ui::hidden_id(id);
        if constexpr (std::is_same_v<T, double>) return ImGui::InputDouble(hidden.c_str(), value, 0.0, 0.0, "%.3f");
        if constexpr (std::is_same_v<T, int>) return ImGui::InputInt(hidden.c_str(), value, 0, 0);
        return false;
    }
}

#endif // ISAACCOYOTE_CONFIG_TAB_INTERNAL_H