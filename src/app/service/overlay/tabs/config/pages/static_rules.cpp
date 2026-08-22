// Created by TsCat on 2026/8/19.
#include <algorithm>
#include <sstream>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"

using namespace app::config;

// Static rules page: collectible/heart strength-source rules; channel modifiers and player filter (shared with events page).

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;
    }

    bool ConfigTab::draw_modifiers(ChannelModifiersConfig& modifiers, const char* id) {
        bool changed = false;
        ImGui::PushID(id);

        const bool compact = ImGui::GetContentRegionAvail().x < ui::metrics::scale(560.0f);
        if (ImGui::BeginTable("modifier_channels", compact ? 1 : 2, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ui::metrics::scale(ImVec2(5.0f, 5.0f)));
            for (int channel = 0; channel < 2; ++channel) {
                auto& slot = channel == 0 ? modifiers.channel_a : modifiers.channel_b;
                ImGui::TableNextColumn();
                ImGui::PushID(channel);

                if (ui::begin_inset_panel("channel_modifier")) {
                    bool enabled = slot.has_value();
                    if (ImGui::Checkbox("##enabled", &enabled)) {
                        if (enabled) slot = ModifierConfig{};
                        else slot.reset();
                        changed = true;
                    }
                    ImGui::SameLine();
                    ImGui::TextUnformatted(channel == 0 ? "通道 A" : "通道 B");
                    ImGui::SameLine();
                    ImGui::TextColored(enabled ? ui::kSuccess : ui::kTextMuted, enabled ? "已启用" : "未启用");

                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(6.0f)));
                    if (slot) {
                        int mode = static_cast<int>(slot->modifier);
                        ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
                        ImGui::TextUnformatted("调整方式");
                        ImGui::PopStyleColor();
                        if (ui::combo("modifier", &mode, detail::kModifiers, IM_ARRAYSIZE(detail::kModifiers))) {
                            slot->modifier = static_cast<StrengthModifier>(mode);
                            changed = true;
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
                        ImGui::TextUnformatted("强度值");
                        ImGui::PopStyleColor();
                        changed |= detail::input("modifier_value", &slot->value);
                    }
                    else {
                        ui::muted_text("先启用这个通道, 再选择调整方式和强度值");
                    }
                }
                ui::end_inset_panel();
                ImGui::PopID();
            }
            ImGui::PopStyleVar();
            ImGui::EndTable();
        }

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_player_filter(PlayerFilterConfig& players, const char* id) {
        bool changed = false;
        ImGui::PushID(id);

        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        if (ui::begin_inset_panel("player_filter_card")) {
            ui::section_header("适用玩家", "选择哪些玩家触发事件时,这条规则会响应");
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
            if (ui::begin_property_table("player_filter")) {
                int scope = static_cast<int>(players.scope);
                ui::property_row("玩家");
                if (ui::combo("scope", &scope, detail::kScopes, IM_ARRAYSIZE(detail::kScopes))) {
                    players.scope = static_cast<PlayerScope>(scope);
                    changed = true;
                }

                if (players.scope == PlayerScope::Specific) {
                    std::string ids;
                    for (const auto& value : players.player_ids) ids += value + "\n";
                    ui::property_row("玩家 ID");
                    if (ui::input_multiline("player_ids", &ids, ImVec2(-1.0f, detail::layout::kPlayerIdsHeight()))) {
                        players.player_ids.clear();
                        std::stringstream stream(ids);
                        std::string line;
                        while (std::getline(stream, line)) {
                            line = detail::trim_line(std::move(line));
                            if (!line.empty()) players.player_ids.push_back(std::move(line));
                        }
                        changed = true;
                    }
                }
                ImGui::EndTable();
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    void ConfigTab::draw_static_rules() {
        auto& rules = state_.draft.game.static_rules;
        if (rules.empty()) state_.selected_static = -1;
        else state_.selected_static = std::clamp(state_.selected_static, 0, static_cast<int>(rules.size()) - 1);

        const bool compact = ImGui::GetContentRegionAvail().x < ui::metrics::scale(690.0f);
        if (ImGui::BeginTable("static_master_detail", compact ? 1 : 2, ImGuiTableFlags_SizingStretchProp |
                              ImGuiTableFlags_BordersInnerV)) {
            if (!compact) {
                ImGui::TableSetupColumn("list", ImGuiTableColumnFlags_WidthFixed, detail::layout::kRuleListWidth());
                ImGui::TableSetupColumn("detail", ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextColumn();
            const float list_height = compact ? detail::layout::kRuleListCompactHeight() : ImGui::GetContentRegionAvail().y;
            if (ui::begin_surface_card("static_list", ImVec2(0.0f, list_height), false)) {
                ui::title_text("规则列表");
                ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
                ImGui::Text("%zu 项", rules.size());
                ImGui::PopStyleColor();

                if (ui::primary_button("新增规则", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(34.0f)))) {
                    rules.push_back({});
                    state_.selected_static = static_cast<int>(rules.size()) - 1;
                    state_.dirty = true;
                }
                if (state_.selected_static >= 0) {
                    ImGui::SameLine();
                    if (ui::danger_button("删除", ImVec2(ui::metrics::scale(72.0f), ui::metrics::scale(34.0f)))) {
                        state_.pending_delete = DeleteTarget::StaticRule;
                        state_.pending_delete_index = state_.selected_static;
                    }
                }
                ImGui::Separator();

                for (int i = 0; i < static_cast<int>(rules.size()); ++i) {
                    const std::string label = rules[i].name.empty()
                                                  ? (rules[i].rule_id.empty() ? "还没取名的规则" : rules[i].rule_id)
                                                  : rules[i].name;
                    const char* hint = std::holds_alternative<CollectibleSourceConfig>(rules[i].source)
                                           ? "按藏品换算强度"
                                           : "按红心换算强度";
                    if (ui::list_navigation_item(std::to_string(i).c_str(), label.c_str(), hint,
                                                 state_.selected_static == i, rules[i].enabled ? "启用" : "停用")) {
                        state_.selected_static = i;
                    }
                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(3.0f)));
                }
            }
            ui::end_surface_card();

            ImGui::TableNextColumn();
            if (compact) ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(10.0f)));
            const float detail_height = compact
                                            ? std::max(ui::metrics::scale(360.0f), ImGui::GetContentRegionAvail().y)
                                            : ImGui::GetContentRegionAvail().y;
            if (ui::begin_surface_card("static_detail", ImVec2(0.0f, detail_height), false)) {
                if (state_.selected_static >= 0 && state_.selected_static < static_cast<int>(rules.size())) {
                    ImGui::PushID(state_.selected_static);
                    auto& rule = rules[state_.selected_static];
                    ui::section_header("规则详情", "选择强度来源、适用玩家,以及通道 A/B 的调整方式");

                    if (ui::begin_property_table("static_meta")) {
                        ui::property_row("显示名称");
                        state_.dirty |= ui::input_text("static_rule_name", "显示名称", &rule.name);
                        ui::property_row("规则 ID");
                        state_.dirty |= ui::input_text("static_rule_id", "唯一标识", &rule.rule_id);
                        ui::property_row("启用");
                        state_.dirty |= ImGui::Checkbox("##static_enabled", &rule.enabled);
                        ImGui::EndTable();
                    }

                    state_.dirty |= draw_player_filter(rule.players, "static_players");

                    if (ui::begin_property_table("static_source_table")) {
                        int source_type = std::holds_alternative<CollectibleSourceConfig>(rule.source) ? 0 : 1;
                        ui::property_row("来源");
                        if (ui::combo("static_source", &source_type, "藏品\0红心\0")) {
                            rule.source = source_type == 0
                                              ? StaticSourceConfig(CollectibleSourceConfig{})
                                              : StaticSourceConfig(HealthSourceConfig{});
                            state_.dirty = true;
                        }
                        ImGui::EndTable();

                        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
                        if (source_type == 0) {
                            auto& source = std::get<CollectibleSourceConfig>(rule.source);
                            for (int quality = 0; quality < 5; ++quality) {
                                ImGui::PushID(quality);
                                if (ImGui::CollapsingHeader(("品质 " + std::to_string(quality)).c_str(),
                                                            quality == 0
                                                                ? ImGuiTreeNodeFlags_DefaultOpen
                                                                : ImGuiTreeNodeFlags_None)) {
                                    auto [it, inserted] = source.modifiers_by_quality.try_emplace(
                                        quality, ChannelModifiersConfig{}
                                    );
                                    if (inserted) state_.dirty = true;
                                    state_.dirty |= draw_modifiers(it->second, "quality_modifiers");
                                }
                                ImGui::PopID();
                            }

                            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
                            ImGui::Separator();
                            ui::section_header("覆写", "覆写某一藏品的强度, 不跟随通用品质规则");
                            for (auto& [collectible, modifiers] : source.override_rule) {
                                ImGui::PushID(collectible);
                                std::string collectible_name = "ID: " + std::to_string(collectible);
                                if (const auto it = std::ranges::find(state_.collectibles, collectible,
                                                                      &CollectibleOption::id);
                                    it != state_.collectibles.end()) {
                                    collectible_name = it->name + "  #" + std::to_string(collectible);
                                }
                                if (ImGui::CollapsingHeader(collectible_name.c_str())) {
                                    state_.dirty |= draw_modifiers(modifiers, "collectible_override");
                                    if (ui::danger_button(
                                        "移除这条覆写", ImVec2(ui::metrics::scale(154.0f), ui::metrics::scale(32.0f)))) {
                                        state_.pending_delete = DeleteTarget::CollectibleOverride;
                                        state_.pending_collectible_id = collectible;
                                    }
                                }
                                ImGui::PopID();
                            }
                            if (ui::primary_button("新建", ImVec2(ui::metrics::scale(116.0f), ui::metrics::scale(34.0f)))) {
                                state_.collectible_picker_open = true;
                                state_.collectible_picker_multi = false;
                                state_.selected_collectible_id = -1;
                                load_collectibles();
                            }
                            draw_collectible_picker(source);
                        }
                        else {
                            ui::section_header("每红心强度", "每缺少一颗红心时, 按这里设置调整强度");
                            state_.dirty |= draw_modifiers(std::get<HealthSourceConfig>(rule.source).per_red_heart,
                                                           "heart_modifiers");
                        }
                    }
                    ImGui::PopID();
                }
                else {
                    ui::empty_state("未选中规则", "从左侧选择规则, 或创建一条新规则");
                }
            }
            ui::end_surface_card();
            ImGui::EndTable();
        }
    }
}