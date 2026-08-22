// Created by TsCat on 2026/8/19.
#include <algorithm>
#include <format>
#include <ranges>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "app/event/catalog.h"
#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"
#include "isaac_spy/isaac/manager.h"

using namespace app::config;

// 事件规则页:触发事件、玩家/实体/道具/胶囊/卡片过滤、动作序列。

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;

        // Event dropdown entries are derived from the event catalog ("无" = None appended at the end)
        inline const auto kEventNames = []() -> std::array<const char*, app::event::kEventCatalog.size()>
        {
            std::array<const char*, app::event::kEventCatalog.size()> names{};
            for (std::size_t i = 0; i < app::event::kEventCatalog.size(); ++i)
                names[i] = app::event::kEventCatalog[i].ui_name.data();
            return names;
        }();

        int event_kind_index(const app::event::EventConfig& event) {
            const app::event::EventType kind = app::event::event_kind_of(event);
            for (std::size_t i = 0; i < app::event::kEventCatalog.size(); ++i)
                if (app::event::kEventCatalog[i].type == kind) return static_cast<int>(i);
            return static_cast<int>(app::event::kEventCatalog.size());
        }

        app::event::EventConfig make_event_config(const int index) {
            if (index >= 0 && index < static_cast<int>(app::event::kEventCatalog.size()) - 1)
                return app::event::make_event_config(app::event::kEventCatalog[static_cast<std::size_t>(index)].type);
            return app::event::NoneEventConfig{};
        }

        std::string resolve_entity_name(const int type, const int variant, const int subtype) {
            auto& manager = isaac_spy::isaac::Manager::get_instance();
            auto* configs = manager.get_entity_config_manager();
            if (!configs) return {};

            const auto config = configs->get_entity(type, variant, subtype);
            if (config.id <= 0) return {};

            std::string name = config.name;
            auto* strings = manager.get_string_table();
            if (strings && name.size() > 1 && name.front() == '#') {
                const char* key = name.c_str() + 1;
                bool ok = false;
                name = strings->get_string("Entities", isaac_spy::isaac::LANGUAGE_CHINESE, key, ok);
                if (!ok || name.empty()) {
                    ok = false;
                    name = strings->get_string("Entities", isaac_spy::isaac::LANGUAGE_ENGLISH, key, ok);
                }
                if (!ok || name.empty()) name = config.name;
            }
            return name;
        }
    }

    void ConfigTab::draw_events() {
        auto& events = state_.draft.game.events;
        if (events.empty()) state_.selected_event = -1;
        else state_.selected_event = std::clamp(state_.selected_event, 0, static_cast<int>(events.size()) - 1);

        const bool compact = ImGui::GetContentRegionAvail().x < ui::metrics::scale(690.0f);
        if (ImGui::BeginTable("event_master_detail", compact ? 1 : 2, ImGuiTableFlags_SizingStretchProp |
                              ImGuiTableFlags_BordersInnerV)) {
            if (!compact) {
                ImGui::TableSetupColumn("list", ImGuiTableColumnFlags_WidthFixed, detail::layout::kRuleListWidth());
                ImGui::TableSetupColumn("detail", ImGuiTableColumnFlags_WidthStretch);
            }

            ImGui::TableNextColumn();
            const float list_height = compact ? detail::layout::kRuleListCompactHeight() : ImGui::GetContentRegionAvail().y;
            if (ui::begin_surface_card("event_list", ImVec2(0.0f, list_height), false)) {
                ui::title_text("规则列表");
                ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
                ImGui::Text("共 %zu 项", events.size());
                ImGui::PopStyleColor();

                if (ui::primary_button("新增规则", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(34.0f)))) {
                    events.push_back({});
                    state_.selected_event = static_cast<int>(events.size()) - 1;
                    state_.dirty = true;
                }
                if (state_.selected_event >= 0) {
                    ImGui::SameLine();
                    if (ui::danger_button("删除", ImVec2(ui::metrics::scale(72.0f), ui::metrics::scale(34.0f)))) {
                        state_.pending_delete = DeleteTarget::EventRule;
                        state_.pending_delete_index = state_.selected_event;
                    }
                }
                ImGui::Separator();

                for (int i = 0; i < static_cast<int>(events.size()); ++i) {
                    const std::string label = events[i].name.empty()
                                                  ? (events[i].rule_id.empty() ? "还没取名的规则" : events[i].rule_id)
                                                  : events[i].name;
                    const int event_index = event_kind_index(events[i].event);
                    if (ui::list_navigation_item(std::to_string(i).c_str(), label.c_str(), kEventNames[event_index],
                                                 state_.selected_event == i, events[i].enabled ? "启用" : "停用")) {
                        state_.selected_event = i;
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
            if (ui::begin_surface_card("event_detail", ImVec2(0.0f, detail_height), false)) {
                if (state_.selected_event >= 0 && state_.selected_event < static_cast<int>(events.size())) {
                    ImGui::PushID(state_.selected_event);
                    auto& event_rule = events[state_.selected_event];
                    ui::section_header("规则详情", "选择触发事件、适用玩家, 以及依次执行的动作");

                    if (ui::begin_property_table("event_meta")) {
                        ui::property_row("显示名称");
                        state_.dirty |= ui::input_text("event_rule_name", "显示名称", &event_rule.name);
                        ui::property_row("规则 ID");
                        state_.dirty |= ui::input_text("event_rule_id", "唯一标识", &event_rule.rule_id);
                        ui::property_row("启用");
                        state_.dirty |= ImGui::Checkbox("##event_enabled", &event_rule.enabled);
                        ImGui::EndTable();
                    }

                    if (ui::begin_property_table("event_type_table")) {
                        int event = event_kind_index(event_rule.event);
                        ui::property_row("触发事件");
                        if (ui::combo("event_type", &event, kEventNames.data(), static_cast<int>(kEventNames.size()))) {
                            event_rule.event = make_event_config(event);
                            state_.dirty = true;
                        }
                        ImGui::EndTable();
                    }

                    std::visit([this, &event_rule](auto& event_config)
                    {
                        using T = std::decay_t<decltype(event_config)>;
                        if constexpr (!std::is_same_v<T, app::event::NoneEventConfig>)
                            state_.dirty |= draw_player_filter(event_config.players, "event_players");
                        if constexpr (std::is_same_v<T, OnHurtConfig>)
                            state_.dirty |= draw_entity_filter(event_config);
                        if constexpr (std::is_same_v<T, OnUseActiveItemConfig>)
                            state_.dirty |= draw_item_filter(event_config);
                        if constexpr (std::is_same_v<T, OnUsePillConfig>)
                            state_.dirty |= draw_pill_filter(event_config);
                        if constexpr (std::is_same_v<T, OnUseCardConfig>)
                            state_.dirty |= draw_card_filter(event_config);
                        if constexpr (std::is_same_v<T, OnRerollGameConfig>) {
                            if (ui::begin_property_table("reroll_table")) {
                                ui::property_row("包含 Rewind 命令");
                                state_.dirty |= ImGui::Checkbox("##include_rewind", &event_config.include_rewind);
                                ImGui::EndTable();
                            }
                        }
                    }, event_rule.event);

                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(12.0f)));
                    ImGui::Separator();
                    ui::section_header("动作序列", "触发后从上到下依次执行");

                    for (int i = 0; i < static_cast<int>(event_rule.actions.size()); ++i) {
                        state_.dirty |= draw_action(event_rule.actions[i], i);
                        if (ui::danger_button(("删除动作##" + std::to_string(i)).c_str(),
                                              ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(32.0f)))) {
                            state_.pending_delete = DeleteTarget::EventAction;
                            state_.pending_delete_index = i;
                        }
                        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
                    }

                    if (ui::primary_button("新增 波形输出", ImVec2(ui::metrics::scale(132.0f), ui::metrics::scale(36.0f)))) {
                        event_rule.actions.emplace_back(PulseEffectConfig{});
                        state_.dirty = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("新增 强度修改", ImVec2(ui::metrics::scale(132.0f), ui::metrics::scale(36.0f)))) {
                        event_rule.actions.emplace_back(StrengthEffectConfig{});
                        state_.dirty = true;
                    }
                    ImGui::PopID();
                }
                else {
                    ui::empty_state("还没有选中规则", "先从左边挑一条规则,再来安排动作");
                }
            }
            ui::end_surface_card();
            ImGui::EndTable();
        }
    }

    bool ConfigTab::draw_action(EventAction& action, const int index) {
        bool changed = false;
        ImGui::PushID(index);

        if (ui::begin_inset_panel("action_card")) {
            const std::string title = "动作 " + std::to_string(index + 1);
            if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ui::begin_property_table("action_properties")) {
                    int type = std::holds_alternative<PulseEffectConfig>(action) ? 0 : 1;
                    ui::property_row("动作类型");
                    if (ui::combo("action_type", &type, "播放波形\0调整强度\0")) {
                        action = type == 0 ? EventAction(PulseEffectConfig{}) : EventAction(StrengthEffectConfig{});
                        changed = true;
                    }

                    if (auto* pulse = std::get_if<PulseEffectConfig>(&action)) {
                        int duration = static_cast<int>(pulse->duration.count());
                        ui::property_row("持续时间(毫秒)");
                        if (detail::input("pulse_duration", &duration)) {
                            pulse->duration = Duration(duration);
                            changed = true;
                        }
                        ui::property_row("波形 A");
                        changed |= detail::draw_pulse_combo("action_pulse_a", state_.draft.pulse_definitions,
                                                            pulse->pulse_a);
                        ui::property_row("波形 B");
                        changed |= detail::draw_pulse_combo("action_pulse_b", state_.draft.pulse_definitions,
                                                            pulse->pulse_b);

                        ui::property_row("触发时震动屏幕");
                        changed |= ImGui::Checkbox("##shake", &pulse->shake);
                        ui::property_row("震动时间(毫秒)");
                        int shake_duration = static_cast<int>(pulse->shake_duration.count());
                        if (detail::input("shake_duration", &shake_duration)) {
                            pulse->shake_duration = Duration(shake_duration);
                            changed = true;
                        }
                    }
                    else {
                        auto& strength = std::get<StrengthEffectConfig>(action);
                        ui::property_row("按固定时长?");
                        changed |= ImGui::Checkbox("##temporary", &strength.temporary);

                        int duration = strength.duration ? static_cast<int>(strength.duration->count()) : 0;
                        ImGui::BeginDisabled(!strength.temporary);
                        ui::property_row("持续时间(毫秒)");
                        if (detail::input("strength_duration", &duration)) {
                            strength.duration = Duration(duration);
                            changed = true;
                        }
                        ImGui::EndDisabled();
                    }
                    ImGui::EndTable();
                }

                if (auto* strength = std::get_if<StrengthEffectConfig>(&action)) {
                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(6.0f)));
                    changed |= draw_modifiers(strength->modifiers, "action_modifiers");
                }
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_entity_key_list(const char* title, std::vector<OnHurtConfig::EntityKey>& keys,
                                         std::vector<std::string>& drafts) {
        bool changed = false;
        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        ImGui::TextUnformatted(title);
        ImGui::PushID("entity_list");

        if (drafts.size() != keys.size()) {
            drafts.clear();
            drafts.reserve(keys.size());
            for (const auto& key : keys) drafts.push_back(key.to_string());
        }

        for (int i = 0; i < static_cast<int>(keys.size()); ++i) {
            ImGui::PushID(i);
            auto& draft = drafts[i];

            ImGui::SetNextItemWidth(ui::metrics::scale(132.0f));
            if (ImGui::InputText("##id", &draft, ImGuiInputTextFlags_None)) {
                OnHurtConfig::EntityKey parsed;
                if (parsed.from_string(detail::trim_line(draft))) {
                    keys[i] = parsed;
                    changed = true;
                }
            }

            const std::string trimmed = detail::trim_line(draft);
            if (!trimmed.empty()) {
                ImGui::SameLine();
                OnHurtConfig::EntityKey parsed;
                ImVec4 chip_color = ui::kError;
                std::string chip_text = "ID 无效";
                if (parsed.from_string(trimmed)) {
                    const std::string name = resolve_entity_name(parsed.type, parsed.variant, parsed.subtype);
                    if (name.empty()) {
                        chip_color = ui::kWarning;
                        chip_text = "未知实体";
                    } else {
                        chip_color = ui::kSuccess;
                        chip_text = name;
                    }
                }
                ui::status_chip("chip", chip_text.c_str(), chip_color);
            }

            ImGui::SameLine();
            if (ui::danger_button("删除", ImVec2(ui::metrics::scale(64.0f), ui::metrics::scale(30.0f)))) {
                keys.erase(keys.begin() + i);
                drafts.erase(drafts.begin() + i);
                changed = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }

        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        if (ui::tonal_button("添加条目", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(30.0f)))) {
            keys.push_back({0, -1, -1});
            drafts.push_back("");
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_entity_filter(config::OnHurtConfig& config) {
        bool changed = false;
        ImGui::PushID("entity_filter");

        if (ui::begin_inset_panel("entity_filter_card")) {
            if (ImGui::CollapsingHeader("按实体过滤", ImGuiTreeNodeFlags_DefaultOpen)) {
                ui::muted_text("受伤来源实体, 输入实体ID, 例如 13.0.0 (苍蝇) -1为通配, 如 20.0.-1 能匹配所有类型的 萌死戳");
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

                if (ui::begin_property_table("entity_filter_mode")) {
                    ui::property_row("白名单模式");
                    changed |= ImGui::Checkbox("##whitelist", &config.whitelist);
                    ImGui::EndTable();
                }

                if (config.whitelist) {
                    changed |= draw_entity_key_list("白名单实体 (仅这些实体触发)", config.whitelist_entities,
                                                    state_.whitelist_entity_drafts);
                } else {
                    changed |= draw_entity_key_list("黑名单实体 (永不触发)", config.blacklist_entities,
                                                    state_.blacklist_entity_drafts);
                }
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_item_key_list(const char* title, std::vector<int>& items, const bool whitelist) {
        bool changed = false;
        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        ImGui::TextUnformatted(title);
        ImGui::PushID("item_list");

        for (int i = 0; i < static_cast<int>(items.size()); ++i) {
            ImGui::PushID(i);
            const int id = items[i];
            const auto it = std::ranges::find(state_.collectibles, id, &CollectibleOption::id);
            if (it != state_.collectibles.end()) {
                ui::status_chip("item_name_chip", it->name.c_str(), ui::kSuccess);
            }
            else if (id <= 0) {
                ui::status_chip("item_name_chip", "ID 无效", ui::kError);
            }
            else {
                ui::status_chip("item_name_chip", "未知道具", ui::kWarning);
            }

            ImGui::SameLine();
            if (ui::danger_button("删除", ImVec2(ui::metrics::scale(64.0f), ui::metrics::scale(30.0f)))) {
                items.erase(items.begin() + i);
                changed = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }

        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        if (ui::tonal_button("选择道具", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(30.0f)))) {
            state_.collectible_picker_open = true;
            state_.collectible_picker_multi = true;
            state_.selected_collectible_id = -1;
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            load_collectibles();
        }
        draw_collectible_picker_modal("选择道具##item_filter_picker",
                                      whitelist ? "添加白名单道具" : "添加黑名单道具",
                                      [&items](const int id) {
                                          return std::ranges::find(items, id) != items.end();
                                      },
                                      true);

        if (state_.picker_confirmed) {
            state_.picker_confirmed = false;
            for (const int id : state_.picker_result_ids) {
                if (std::ranges::find(items, id) == items.end())
                    items.push_back(id);
            }
            state_.picker_result_ids.clear();
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_item_filter(config::OnUseActiveItemConfig& config) {
        bool changed = false;
        ImGui::PushID("item_filter");

        if (ui::begin_inset_panel("item_filter_card")) {
            if (ImGui::CollapsingHeader("按主动道具过滤", ImGuiTreeNodeFlags_DefaultOpen)) {
                ui::muted_text("按主动道具 ID 过滤, 使用道具时只有匹配的道具才会触发本条规则");
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

                if (ui::begin_property_table("item_filter_mode")) {
                    ui::property_row("白名单模式");
                    changed |= ImGui::Checkbox("##whitelist", &config.whitelist);
                    ImGui::EndTable();
                }

                if (config.whitelist) {
                    changed |= draw_item_key_list("白名单道具 (仅这些道具触发)", config.whitelist_items, true);
                } else {
                    changed |= draw_item_key_list("黑名单道具 (永不触发)", config.blacklist_items, false);
                }
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_pill_key_list(const char* title, std::vector<int>& pills, const bool whitelist) {
        bool changed = false;
        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        ImGui::TextUnformatted(title);
        ImGui::PushID("pill_list");

        for (int i = 0; i < static_cast<int>(pills.size()); ++i) {
            ImGui::PushID(i);
            const int id = pills[i];
            const auto it = std::ranges::find(state_.pills, id, &CollectibleOption::id);
            if (it != state_.pills.end()) {
                ui::status_chip("pill_name_chip", it->name.c_str(), ui::kSuccess);
            }
            else if (id <= 0) {
                ui::status_chip("pill_name_chip", "ID 无效", ui::kError);
            }
            else {
                ui::status_chip("pill_name_chip", "未知胶囊", ui::kWarning);
            }

            ImGui::SameLine();
            if (ui::danger_button("删除", ImVec2(ui::metrics::scale(64.0f), ui::metrics::scale(30.0f)))) {
                pills.erase(pills.begin() + i);
                changed = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }

        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        if (ui::tonal_button("选择胶囊效果", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(30.0f)))) {
            state_.collectible_picker_open = true;
            state_.collectible_picker_multi = true;
            state_.selected_collectible_id = -1;
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            load_pills();
        }
        draw_pill_picker_modal("选择胶囊效果##pill_filter_picker",
                               whitelist ? "添加白名单胶囊效果" : "添加黑名单胶囊效果",
                               [&pills](const int id) {
                                   return std::ranges::find(pills, id) != pills.end();
                               });

        if (state_.picker_confirmed) {
            state_.picker_confirmed = false;
            for (const int id : state_.picker_result_ids) {
                if (std::ranges::find(pills, id) == pills.end())
                    pills.push_back(id);
            }
            state_.picker_result_ids.clear();
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_pill_filter(config::OnUsePillConfig& config) {
        bool changed = false;
        ImGui::PushID("pill_filter");

        if (ui::begin_inset_panel("pill_filter_card")) {
            if (ImGui::CollapsingHeader("按胶囊效果过滤", ImGuiTreeNodeFlags_DefaultOpen)) {
                ui::muted_text("按胶囊效果 ID 过滤, 使用胶囊时只有匹配的效果才会触发本条规则");
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

                if (ui::begin_property_table("pill_filter_mode")) {
                    ui::property_row("白名单模式");
                    changed |= ImGui::Checkbox("##whitelist", &config.whitelist);
                    ImGui::EndTable();
                }

                if (config.whitelist) {
                    changed |= draw_pill_key_list("白名单胶囊效果 (仅这些效果触发)", config.whitelist_pills, true);
                } else {
                    changed |= draw_pill_key_list("黑名单胶囊效果 (永不触发)", config.blacklist_pills, false);
                }
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_card_key_list(const char* title, std::vector<int>& cards, const bool whitelist) {
        bool changed = false;
        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        ImGui::TextUnformatted(title);
        ImGui::PushID("card_list");

        for (int i = 0; i < static_cast<int>(cards.size()); ++i) {
            ImGui::PushID(i);
            const int id = cards[i];
            const auto it = std::ranges::find(state_.cards, id, &CollectibleOption::id);
            if (it != state_.cards.end()) {
                ui::status_chip("card_name_chip", it->name.c_str(), ui::kSuccess);
            }
            else if (id <= 0) {
                ui::status_chip("card_name_chip", "ID 无效", ui::kError);
            }
            else {
                ui::status_chip("card_name_chip", "未知卡片", ui::kWarning);
            }

            ImGui::SameLine();
            if (ui::danger_button("删除", ImVec2(ui::metrics::scale(64.0f), ui::metrics::scale(30.0f)))) {
                cards.erase(cards.begin() + i);
                changed = true;
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }

        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
        if (ui::tonal_button("选择卡片", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(30.0f)))) {
            state_.collectible_picker_open = true;
            state_.collectible_picker_multi = true;
            state_.selected_collectible_id = -1;
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            load_cards();
        }
        draw_card_picker_modal("选择卡片##card_filter_picker",
                               whitelist ? "添加白名单卡片" : "添加黑名单卡片",
                               [&cards](const int id) {
                                   return std::ranges::find(cards, id) != cards.end();
                               });

        if (state_.picker_confirmed) {
            state_.picker_confirmed = false;
            for (const int id : state_.picker_result_ids) {
                if (std::ranges::find(cards, id) == cards.end())
                    cards.push_back(id);
            }
            state_.picker_result_ids.clear();
            changed = true;
        }

        ImGui::PopID();
        return changed;
    }

    bool ConfigTab::draw_card_filter(config::OnUseCardConfig& config) {
        bool changed = false;
        ImGui::PushID("card_filter");

        if (ui::begin_inset_panel("card_filter_card")) {
            if (ImGui::CollapsingHeader("按卡片过滤", ImGuiTreeNodeFlags_DefaultOpen)) {
                ui::muted_text("按卡片 ID 过滤, 使用卡片时只有匹配的卡片才会触发本条规则");
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

                if (ui::begin_property_table("card_filter_mode")) {
                    ui::property_row("白名单模式");
                    changed |= ImGui::Checkbox("##whitelist", &config.whitelist);
                    ImGui::EndTable();
                }

                if (config.whitelist) {
                    changed |= draw_card_key_list("白名单卡片 (仅这些卡片触发)", config.whitelist_cards, true);
                } else {
                    changed |= draw_card_key_list("黑名单卡片 (永不触发)", config.blacklist_cards, false);
                }
            }
        }
        ui::end_inset_panel();

        ImGui::PopID();
        return changed;
    }

    void ConfigTab::draw_collectible_picker(CollectibleSourceConfig& source) {
        draw_collectible_picker_modal("添加藏品规则##collectible_picker", "添加覆写",
                                      [&source](const int id) { return source.override_rule.contains(id); });

        if (!state_.picker_confirmed) return;
        state_.picker_confirmed = false;
        if (state_.picker_result_ids.empty()) return;
        source.override_rule.try_emplace(state_.picker_result_ids.front());
        state_.dirty = true;
        state_.selected_collectible_id.reset();
        state_.manual_collectible_id = 0;
    }
}