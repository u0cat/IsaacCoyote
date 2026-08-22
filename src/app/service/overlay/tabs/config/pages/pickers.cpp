// Created by TsCat on 2026/8/19.
#include <algorithm>
#include <functional>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"
#include "isaac_spy/isaac/enums.h"
#include "isaac_spy/isaac/manager.h"

// 藏品/胶囊/卡片选择器弹窗与名称目录加载(共享 UI 状态见 ConfigTabState)。

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;
    }

    void ConfigTab::load_collectibles() {
        state_.collectibles.clear();
        auto& manager = isaac_spy::isaac::Manager::get_instance();
        auto* item_configs = manager.get_item_config_manager();
        if (!item_configs) return;

        auto* strings = manager.get_string_table();
        for (const auto& [id, config] : item_configs->get_all_collectibles()) {
            if (id <= 0) continue;
            std::string name = config.name;
            if (strings && !config.name.empty() && config.name.front() == '#') {
                bool success = false;
                name = strings->get_string("Items", isaac_spy::isaac::LANGUAGE_CHINESE,
                                           config.name.c_str() + 1, success);
                if (!success || name.empty()) name = config.name;
            }
            state_.collectibles.push_back({id, config.quality, std::move(name), config.name});
        }
        std::ranges::sort(state_.collectibles, {}, &CollectibleOption::id);
    }

    void ConfigTab::load_pills() {
        state_.pills.clear();
        auto& manager = isaac_spy::isaac::Manager::get_instance();
        auto* item_configs = manager.get_item_config_manager();
        if (!item_configs) return;

        auto* strings = manager.get_string_table();
        for (const auto& [id, config] : item_configs->get_all_pills()) {
            if (id < 0) continue;
            std::string name = config.name;
            if (strings && !config.name.empty() && config.name.front() == '#') {
                bool success = false;
                name = strings->get_string("PocketItems", isaac_spy::isaac::LANGUAGE_CHINESE,
                                           config.name.c_str() + 1, success);
                if (!success || name.empty()) name = config.name;
            }
            state_.pills.push_back({id, config.quality, std::move(name), config.name});
        }
        std::ranges::sort(state_.pills, {}, &CollectibleOption::id);
    }

    void ConfigTab::load_cards() {
        state_.cards.clear();
        auto& manager = isaac_spy::isaac::Manager::get_instance();
        auto* item_configs = manager.get_item_config_manager();
        if (!item_configs) return;

        auto* strings = manager.get_string_table();
        for (const auto& [id, config] : item_configs->get_all_cards()) {
            if (id <= 0) continue;
            std::string name = config.name;
            if (strings && !config.name.empty() && config.name.front() == '#') {
                bool success = false;
                name = strings->get_string("PocketItems", isaac_spy::isaac::LANGUAGE_CHINESE,
                                           config.name.c_str() + 1, success);
                if (!success || name.empty()) name = config.name;
            }
            state_.cards.push_back({id, config.quality, std::move(name), config.name});
        }
        std::ranges::sort(state_.cards, {}, &CollectibleOption::id);
    }

    void ConfigTab::draw_collectible_picker_modal(const char* popup_id, const char* title,
                                                  const std::function<bool(int)>& is_taken,
                                                  const bool only_active) {
        if (state_.collectible_picker_open) {
            state_.collectible_picker_open = false;
            state_.selected_collectible_id.reset();
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            state_.picker_confirmed = false;
            state_.picker_result_ids.clear();

            ImGui::OpenPopup(popup_id);
        }

        const float padding = ui::metrics::scale(24.0f);
        const float cancel_width = ui::metrics::scale(92.0f);
        const float submit_width = ui::metrics::scale(132.0f);
        const float button_height = ui::metrics::scale(36.0f);
        const float button_spacing = ImGui::GetStyle().ItemSpacing.x;

        const float buttons_width =
            cancel_width +
            button_spacing +
            submit_width;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkCenter(),ImGuiCond_Always,ImVec2(0.5f, 0.5f));

        ImGui::SetNextWindowSize(ImVec2(ui::metrics::kModalWidth, 0.0f), ImGuiCond_Appearing);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui::metrics::kCardRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ui::metrics::scale(1.0f));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(ui::kSurface.x, ui::kSurface.y, ui::kSurface.z, 0.76f));

        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar;

        if (ImGui::BeginPopupModal(popup_id, nullptr, flags)) {
            // header
            ui::title_text(title);

            ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(state_.collectible_picker_multi ? "搜索或手动输入藏品 ID, 可多选" : "搜索或手动输入藏品 ID");
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            // content
            auto configs = isaac_spy::isaac::Manager::get_instance().get_item_config_manager();
            if (!configs) return;
            auto all_coll_vec = configs->get_all_collectibles();

            ui::input_text("debug_collectible_query", "输入 ID 或藏品名字", &state_.collectible_query);
            constexpr const char* qualities[] = {"全部品质", "品质 0", "品质 1", "品质 2", "品质 3", "品质 4"};
            int quality = state_.collectible_quality + 1;
            if (ui::combo("debug_collectible_quality", &quality, qualities, IM_ARRAYSIZE(qualities))) {
                state_.collectible_quality = quality - 1;
            }

            const float table_height = std::min(ui::table_height(8),ImGui::GetContentRegionAvail().y);
            if (ui::begin_data_table("collectible_table", 4, ImVec2(0.0f, table_height))) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("藏品", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableSetupColumn("品质", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableSetupColumn("状态", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableHeadersRow();

                for (auto [coll_id, config] : all_coll_vec) {
                    if (state_.collectible_quality >= 0 && config.quality != state_.collectible_quality) continue;
                    // The item picker for active-item rules only lists ACTIVE items.
                    if (only_active && config.type != isaac_spy::isaac::enums::ITEM_ACTIVE) continue;

                    std::string_view key = config.name;
                    if (!key.empty() && key[0] == '#') {
                        key.remove_prefix(1);
                    }
                    const bool taken = is_taken(coll_id);
                    const bool selected = state_.selected_collectible_id.has_value() &&
                                          *state_.selected_collectible_id == coll_id;
                    const bool multi_selected = std::ranges::find(state_.picker_selected_ids, coll_id) !=
                                                state_.picker_selected_ids.end();

                    bool success = false;
                    auto name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "Items", isaac_spy::isaac::LANGUAGE_CHINESE, key.data(), success);
                    if (!success) name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "Items", isaac_spy::isaac::LANGUAGE_ENGLISH, key.data(), success);
                    if (!success) name = config.name;

                    std::string searchable = name + " " + config.name + " " + std::to_string(coll_id);
                    std::string query = state_.collectible_query;
                    std::ranges::transform(searchable, searchable.begin(), [](unsigned char c) { return std::tolower(c); });
                    std::ranges::transform(query, query.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (!query.empty() && !searchable.contains(query)) continue;

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(coll_id);
                    ImGui::BeginDisabled(taken);
                    if (ImGui::Selectable(name.c_str(),
                                          state_.collectible_picker_multi ? multi_selected : selected,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                        if (state_.collectible_picker_multi) {
                            if (auto it = std::ranges::find(state_.picker_selected_ids, coll_id);
                                it != state_.picker_selected_ids.end()) {
                                state_.picker_selected_ids.erase(it);
                            }
                            else {
                                state_.picker_selected_ids.push_back(coll_id);
                            }
                        }
                        else {
                            state_.selected_collectible_id = coll_id;
                            state_.manual_collectible_id = 0;
                        }
                    }
                    ImGui::EndDisabled();
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", coll_id);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", config.quality);
                    ImGui::TableNextColumn();
                    if (taken) {
                        ImGui::TextColored(ui::kTextMuted, "已存在");
                    }
                    else {
                        ImGui::TextColored(state_.collectible_picker_multi ? (multi_selected ? ui::kSuccess : ui::kTextMuted)
                                                                           : (selected ? ui::kSuccess : ui::kTextMuted),
                                           state_.collectible_picker_multi ? (multi_selected ? "已选" : "可选")
                                                                           : (selected ? "已选" : "可添加"));
                    }
                }
                ui::end_data_table();
            }

            ImGui::Dummy(ImVec2{0.0f, ui::metrics::scale(8.0f)});

            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("手动输入藏品 ID");

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ui::metrics::scale(120.0f));
            if (ImGui::InputInt("##manual_collectible_id", &state_.manual_collectible_id, 0, 0)) {
                state_.manual_collectible_id = std::max(0, state_.manual_collectible_id);

                if (state_.manual_collectible_id > 0 && !state_.collectible_picker_multi) {
                    state_.selected_collectible_id.reset();
                }
            }
            if (state_.collectible_picker_multi && state_.manual_collectible_id > 0) {
                ImGui::SameLine();
                if (ui::tonal_button("加入", ImVec2(ui::metrics::scale(64.0f), button_height))) {
                    if (std::ranges::find(state_.picker_selected_ids, state_.manual_collectible_id) ==
                        state_.picker_selected_ids.end()) {
                        state_.picker_selected_ids.push_back(state_.manual_collectible_id);
                    }
                }
            }
            if (!state_.collectible_picker_multi && state_.manual_collectible_id > 0 &&
                is_taken(state_.manual_collectible_id)) {
                ImGui::SameLine();
                ImGui::TextColored(ui::kPrimaryMuted, "该藏品已经存在覆写");
            }

            // footer
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            const float button_group_x =ImGui::GetCursorPosX() +
                std::max(0.0f,ImGui::GetContentRegionAvail().x - buttons_width);

            int chosen_id = 0;
            if (state_.manual_collectible_id > 0) {
                chosen_id = state_.manual_collectible_id;
            }
            else if (state_.selected_collectible_id.has_value()) {
                chosen_id = *state_.selected_collectible_id;
            }

            const bool has_selection = state_.collectible_picker_multi
                                           ? !state_.picker_selected_ids.empty()
                                           : (chosen_id != 0 && !is_taken(chosen_id));
            ImGui::SetCursorPosX(button_group_x);
            if (ui::tonal_button("取消",ImVec2(cancel_width, button_height))) ImGui::CloseCurrentPopup();
            ImGui::SameLine(0.0f, button_spacing);

            ImGui::BeginDisabled(!has_selection);
            if (ui::primary_button(state_.collectible_picker_multi ? "添加" : "添加",
                                   ImVec2(submit_width, button_height))) {
                state_.picker_result_ids = state_.collectible_picker_multi
                                               ? state_.picker_selected_ids
                                               : std::vector<int>{chosen_id};
                state_.picker_confirmed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }

    void ConfigTab::draw_pill_picker_modal(const char* popup_id, const char* title,
                                           const std::function<bool(int)>& is_taken) {
        if (state_.collectible_picker_open) {
            state_.collectible_picker_open = false;
            state_.selected_collectible_id.reset();
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            state_.picker_confirmed = false;
            state_.picker_result_ids.clear();

            ImGui::OpenPopup(popup_id);
        }

        const float padding = ui::metrics::scale(24.0f);
        const float cancel_width = ui::metrics::scale(92.0f);
        const float submit_width = ui::metrics::scale(132.0f);
        const float button_height = ui::metrics::scale(36.0f);
        const float button_spacing = ImGui::GetStyle().ItemSpacing.x;

        const float buttons_width =
            cancel_width +
            button_spacing +
            submit_width;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkCenter(),ImGuiCond_Always,ImVec2(0.5f, 0.5f));

        ImGui::SetNextWindowSize(ImVec2(ui::metrics::kModalWidth, 0.0f), ImGuiCond_Appearing);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui::metrics::kCardRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ui::metrics::scale(1.0f));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(ui::kSurface.x, ui::kSurface.y, ui::kSurface.z, 0.76f));

        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar;

        if (ImGui::BeginPopupModal(popup_id, nullptr, flags)) {
            // header
            ui::title_text(title);

            ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(state_.collectible_picker_multi ? "搜索或手动输入胶囊效果 ID, 可多选" : "搜索或手动输入胶囊效果 ID");
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            // content
            auto configs = isaac_spy::isaac::Manager::get_instance().get_item_config_manager();
            if (!configs) return;
            auto all_pills_vec = configs->get_all_pills();

            ui::input_text("debug_pill_query", "输入 ID 或胶囊效果名字", &state_.collectible_query);

            const float table_height = std::min(ui::table_height(8),ImGui::GetContentRegionAvail().y);
            if (ui::begin_data_table("pill_table", 3, ImVec2(0.0f, table_height))) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("胶囊效果", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableSetupColumn("状态", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableHeadersRow();

                for (auto [pill_id, config] : all_pills_vec) {
                    std::string_view key = config.name;
                    if (!key.empty() && key[0] == '#') {
                        key.remove_prefix(1);
                    }
                    const bool taken = is_taken(pill_id);
                    const bool selected = state_.selected_collectible_id.has_value() &&
                                          *state_.selected_collectible_id == pill_id;
                    const bool multi_selected = std::ranges::find(state_.picker_selected_ids, pill_id) !=
                                                state_.picker_selected_ids.end();

                    bool success = false;
                    auto name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "PocketItems", isaac_spy::isaac::LANGUAGE_CHINESE, key.data(), success);
                    if (!success) name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "PocketItems", isaac_spy::isaac::LANGUAGE_ENGLISH, key.data(), success);
                    if (!success) name = config.name;

                    std::string searchable = name + " " + config.name + " " + std::to_string(pill_id);
                    std::string query = state_.collectible_query;
                    std::ranges::transform(searchable, searchable.begin(), [](unsigned char c) { return std::tolower(c); });
                    std::ranges::transform(query, query.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (!query.empty() && !searchable.contains(query)) continue;

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(pill_id);
                    ImGui::BeginDisabled(taken);
                    if (ImGui::Selectable(name.c_str(),
                                          state_.collectible_picker_multi ? multi_selected : selected,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                        if (state_.collectible_picker_multi) {
                            if (auto it = std::ranges::find(state_.picker_selected_ids, pill_id);
                                it != state_.picker_selected_ids.end()) {
                                state_.picker_selected_ids.erase(it);
                            }
                            else {
                                state_.picker_selected_ids.push_back(pill_id);
                            }
                        }
                        else {
                            state_.selected_collectible_id = pill_id;
                            state_.manual_collectible_id = 0;
                        }
                    }
                    ImGui::EndDisabled();
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", pill_id);
                    ImGui::TableNextColumn();
                    if (taken) {
                        ImGui::TextColored(ui::kTextMuted, "已存在");
                    }
                    else {
                        ImGui::TextColored(state_.collectible_picker_multi ? (multi_selected ? ui::kSuccess : ui::kTextMuted)
                                                                           : (selected ? ui::kSuccess : ui::kTextMuted),
                                           state_.collectible_picker_multi ? (multi_selected ? "已选" : "可选")
                                                                           : (selected ? "已选" : "可添加"));
                    }
                }
                ui::end_data_table();
            }

            ImGui::Dummy(ImVec2{0.0f, ui::metrics::scale(8.0f)});

            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("手动输入胶囊效果 ID");

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ui::metrics::scale(120.0f));
            if (ImGui::InputInt("##manual_pill_id", &state_.manual_collectible_id, 0, 0)) {
                state_.manual_collectible_id = std::max(0, state_.manual_collectible_id);

                if (state_.manual_collectible_id > 0 && !state_.collectible_picker_multi) {
                    state_.selected_collectible_id.reset();
                }
            }
            if (state_.collectible_picker_multi && state_.manual_collectible_id > 0) {
                ImGui::SameLine();
                if (ui::tonal_button("加入", ImVec2(ui::metrics::scale(64.0f), button_height))) {
                    if (std::ranges::find(state_.picker_selected_ids, state_.manual_collectible_id) ==
                        state_.picker_selected_ids.end()) {
                        state_.picker_selected_ids.push_back(state_.manual_collectible_id);
                    }
                }
            }
            if (!state_.collectible_picker_multi && state_.manual_collectible_id > 0 &&
                is_taken(state_.manual_collectible_id)) {
                ImGui::SameLine();
                ImGui::TextColored(ui::kPrimaryMuted, "该胶囊效果已经存在");
            }

            // footer
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            const float button_group_x =ImGui::GetCursorPosX() +
                std::max(0.0f,ImGui::GetContentRegionAvail().x - buttons_width);

            int chosen_id = 0;
            if (state_.manual_collectible_id > 0) {
                chosen_id = state_.manual_collectible_id;
            }
            else if (state_.selected_collectible_id.has_value()) {
                chosen_id = *state_.selected_collectible_id;
            }

            const bool has_selection = state_.collectible_picker_multi
                                           ? !state_.picker_selected_ids.empty()
                                           : (chosen_id != 0 && !is_taken(chosen_id));
            ImGui::SetCursorPosX(button_group_x);
            if (ui::tonal_button("取消",ImVec2(cancel_width, button_height))) ImGui::CloseCurrentPopup();
            ImGui::SameLine(0.0f, button_spacing);

            ImGui::BeginDisabled(!has_selection);
            if (ui::primary_button(state_.collectible_picker_multi ? "添加" : "添加",
                                   ImVec2(submit_width, button_height))) {
                state_.picker_result_ids = state_.collectible_picker_multi
                                               ? state_.picker_selected_ids
                                               : std::vector<int>{chosen_id};
                state_.picker_confirmed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }

    void ConfigTab::draw_card_picker_modal(const char* popup_id, const char* title,
                                           const std::function<bool(int)>& is_taken) {
        if (state_.collectible_picker_open) {
            state_.collectible_picker_open = false;
            state_.selected_collectible_id.reset();
            state_.manual_collectible_id = 0;
            state_.picker_selected_ids.clear();
            state_.picker_confirmed = false;
            state_.picker_result_ids.clear();

            ImGui::OpenPopup(popup_id);
        }

        const float padding = ui::metrics::scale(24.0f);
        const float cancel_width = ui::metrics::scale(92.0f);
        const float submit_width = ui::metrics::scale(132.0f);
        const float button_height = ui::metrics::scale(36.0f);
        const float button_spacing = ImGui::GetStyle().ItemSpacing.x;

        const float buttons_width =
            cancel_width +
            button_spacing +
            submit_width;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkCenter(),ImGuiCond_Always,ImVec2(0.5f, 0.5f));

        ImGui::SetNextWindowSize(ImVec2(ui::metrics::kModalWidth, 0.0f), ImGuiCond_Appearing);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, ui::metrics::kCardRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, ui::metrics::scale(1.0f));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(ui::kSurface.x, ui::kSurface.y, ui::kSurface.z, 0.76f));

        constexpr ImGuiWindowFlags flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoTitleBar;

        if (ImGui::BeginPopupModal(popup_id, nullptr, flags)) {
            // header
            ui::title_text(title);

            ImGui::PushStyleColor(ImGuiCol_Text, ui::kTextMuted);
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(state_.collectible_picker_multi ? "搜索或手动输入卡片 ID, 可多选" : "搜索或手动输入卡片 ID");
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            // content
            auto configs = isaac_spy::isaac::Manager::get_instance().get_item_config_manager();
            if (!configs) return;
            auto all_cards_vec = configs->get_all_cards();

            ui::input_text("debug_card_query", "输入 ID 或卡片名字", &state_.collectible_query);

            const float table_height = std::min(ui::table_height(8),ImGui::GetContentRegionAvail().y);
            if (ui::begin_data_table("card_table", 3, ImVec2(0.0f, table_height))) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("卡片", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableSetupColumn("状态", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
                ImGui::TableHeadersRow();

                for (auto [card_id, config] : all_cards_vec) {
                    std::string_view key = config.name;
                    if (!key.empty() && key[0] == '#') {
                        key.remove_prefix(1);
                    }
                    const bool taken = is_taken(card_id);
                    const bool selected = state_.selected_collectible_id.has_value() &&
                                          *state_.selected_collectible_id == card_id;
                    const bool multi_selected = std::ranges::find(state_.picker_selected_ids, card_id) !=
                                                state_.picker_selected_ids.end();

                    bool success = false;
                    auto name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "PocketItems", isaac_spy::isaac::LANGUAGE_CHINESE, key.data(), success);
                    if (!success) name = isaac_spy::isaac::Manager::get_instance().get_string_table()->get_string(
                        "PocketItems", isaac_spy::isaac::LANGUAGE_ENGLISH, key.data(), success);
                    if (!success) name = config.name;

                    std::string searchable = name + " " + config.name + " " + std::to_string(card_id);
                    std::string query = state_.collectible_query;
                    std::ranges::transform(searchable, searchable.begin(), [](unsigned char c) { return std::tolower(c); });
                    std::ranges::transform(query, query.begin(), [](unsigned char c) { return std::tolower(c); });
                    if (!query.empty() && !searchable.contains(query)) continue;

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::PushID(card_id);
                    ImGui::BeginDisabled(taken);
                    if (ImGui::Selectable(name.c_str(),
                                          state_.collectible_picker_multi ? multi_selected : selected,
                                          ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                        if (state_.collectible_picker_multi) {
                            if (auto it = std::ranges::find(state_.picker_selected_ids, card_id);
                                it != state_.picker_selected_ids.end()) {
                                state_.picker_selected_ids.erase(it);
                            }
                            else {
                                state_.picker_selected_ids.push_back(card_id);
                            }
                        }
                        else {
                            state_.selected_collectible_id = card_id;
                            state_.manual_collectible_id = 0;
                        }
                    }
                    ImGui::EndDisabled();
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", card_id);
                    ImGui::TableNextColumn();
                    if (taken) {
                        ImGui::TextColored(ui::kTextMuted, "已存在");
                    }
                    else {
                        ImGui::TextColored(state_.collectible_picker_multi ? (multi_selected ? ui::kSuccess : ui::kTextMuted)
                                                                           : (selected ? ui::kSuccess : ui::kTextMuted),
                                           state_.collectible_picker_multi ? (multi_selected ? "已选" : "可选")
                                                                           : (selected ? "已选" : "可添加"));
                    }
                }
                ui::end_data_table();
            }

            ImGui::Dummy(ImVec2{0.0f, ui::metrics::scale(8.0f)});

            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("手动输入卡片 ID");

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ui::metrics::scale(120.0f));
            if (ImGui::InputInt("##manual_card_id", &state_.manual_collectible_id, 0, 0)) {
                state_.manual_collectible_id = std::max(0, state_.manual_collectible_id);

                if (state_.manual_collectible_id > 0 && !state_.collectible_picker_multi) {
                    state_.selected_collectible_id.reset();
                }
            }
            if (state_.collectible_picker_multi && state_.manual_collectible_id > 0) {
                ImGui::SameLine();
                if (ui::tonal_button("加入", ImVec2(ui::metrics::scale(64.0f), button_height))) {
                    if (std::ranges::find(state_.picker_selected_ids, state_.manual_collectible_id) ==
                        state_.picker_selected_ids.end()) {
                        state_.picker_selected_ids.push_back(state_.manual_collectible_id);
                    }
                }
            }
            if (!state_.collectible_picker_multi && state_.manual_collectible_id > 0 &&
                is_taken(state_.manual_collectible_id)) {
                ImGui::SameLine();
                ImGui::TextColored(ui::kPrimaryMuted, "该卡片已经存在");
            }

            // footer
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(8.0f)));

            const float button_group_x =ImGui::GetCursorPosX() +
                std::max(0.0f,ImGui::GetContentRegionAvail().x - buttons_width);

            int chosen_id = 0;
            if (state_.manual_collectible_id > 0) {
                chosen_id = state_.manual_collectible_id;
            }
            else if (state_.selected_collectible_id.has_value()) {
                chosen_id = *state_.selected_collectible_id;
            }

            const bool has_selection = state_.collectible_picker_multi
                                           ? !state_.picker_selected_ids.empty()
                                           : (chosen_id != 0 && !is_taken(chosen_id));
            ImGui::SetCursorPosX(button_group_x);
            if (ui::tonal_button("取消",ImVec2(cancel_width, button_height))) ImGui::CloseCurrentPopup();
            ImGui::SameLine(0.0f, button_spacing);

            ImGui::BeginDisabled(!has_selection);
            if (ui::primary_button(state_.collectible_picker_multi ? "添加" : "添加",
                                   ImVec2(submit_width, button_height))) {
                state_.picker_result_ids = state_.collectible_picker_multi
                                               ? state_.picker_selected_ids
                                               : std::vector<int>{chosen_id};
                state_.picker_confirmed = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);
    }
}