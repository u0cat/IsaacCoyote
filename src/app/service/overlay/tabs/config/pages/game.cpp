// Created by TsCat on 2026/8/19.
#include <imgui.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"

using namespace app::config;

// 基础强度页:强度/上限、持续模式、缓降、爬升。

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;
    }

    void ConfigTab::draw_game() {
        auto& game = state_.draft.game;
        const bool two_columns = !ui::is_compact();

        if (ImGui::BeginTable("game_cards", two_columns ? 2 : 1, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ui::metrics::scale(ImVec2(6.0f, 6.0f)));

            ImGui::TableNextColumn();
            if (ui::begin_surface_card("strength_card")) {
                ui::section_header("基础强度", "设置通道 A 和 B 的起始强度与安全上限");
                ImGui::Dummy(ImVec2(0.0f, 4.0f));
                if (ui::begin_property_table("strength_properties")) {
                    ui::property_row("基础 A");
                    state_.dirty |= detail::input("base_a", &game.strength.base_a);
                    ui::property_row("基础 B");
                    state_.dirty |= detail::input("base_b", &game.strength.base_b);
                    ui::property_row("上限 A");
                    state_.dirty |= detail::input("limit_a", &game.strength.limit_a);
                    ui::property_row("上限 B");
                    state_.dirty |= detail::input("limit_b", &game.strength.limit_b);
                    ImGui::EndTable();
                }
            }
            ui::end_surface_card();

            ImGui::TableNextColumn();
            if (ui::begin_surface_card("constant_card")) {
                ui::section_header("持续模式", "启用后通道 A 和 B 持续播放各自指定的波形");
                ImGui::Dummy(ImVec2(0.0f, 4.0f));
                if (ui::begin_property_table("constant_properties")) {
                    ui::property_row("启用");
                    state_.dirty |= ImGui::Checkbox("##constant_enabled", &game.constant_mode.enabled);

                    ImGui::BeginDisabled(!game.constant_mode.enabled);
                    ui::property_row("波形 A");
                    state_.dirty |= detail::draw_pulse_combo("constant_pulse_a", state_.draft.pulse_definitions,
                                                             game.constant_mode.pulse_a);
                    ui::property_row("波形 B");
                    state_.dirty |= detail::draw_pulse_combo("constant_pulse_b", state_.draft.pulse_definitions,
                                                             game.constant_mode.pulse_b);
                    ImGui::EndDisabled();
                    ImGui::EndTable();
                }
            }
            ui::end_surface_card();

            ImGui::TableNextColumn();
            if (ui::begin_surface_card("decay_card")) {
                ui::section_header("强度缓降", "强度高于目标值时, 按设定间隔逐步降低");
                ImGui::Dummy(ImVec2(0.0f, 4.0f));
                if (ui::begin_property_table("decay_properties")) {
                    ui::property_row("启用");
                    state_.dirty |= ImGui::Checkbox("##decay_enabled", &game.decay.enabled);

                    ImGui::BeginDisabled(!game.decay.enabled);
                    int interval = static_cast<int>(game.decay.interval.count());
                    ui::property_row("间隔(毫秒)");
                    if (detail::input("decay_interval", &interval)) {
                        game.decay.interval = Duration(interval);
                        state_.dirty = true;
                    }
                    ui::property_row("每次降低");
                    state_.dirty |= detail::input("decay_amount", &game.decay.amount);
                    ImGui::EndDisabled();
                    ImGui::EndTable();
                }
            }
            ui::end_surface_card();

            ImGui::TableNextColumn();
            if (ui::begin_surface_card("climb_card")) {
                ui::section_header("强度爬升", "强度低于目标值时, 按设定间隔逐步升高");
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
                if (ui::begin_property_table("climb_properties")) {
                    ui::property_row("启用");
                    state_.dirty |= ImGui::Checkbox("##climb_enabled", &game.climb.enabled);

                    ImGui::BeginDisabled(!game.climb.enabled);
                    int interval = static_cast<int>(game.climb.interval.count());
                    ui::property_row("间隔(毫秒)");
                    if (detail::input("climb_interval", &interval)) {
                        game.climb.interval = Duration(interval);
                        state_.dirty = true;
                    }
                    ui::property_row("每次增加");
                    state_.dirty |= detail::input("climb_amount", &game.climb.amount);
                    ImGui::EndDisabled();
                    ImGui::EndTable();
                }
            }
            ui::end_surface_card();

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }
    }
}