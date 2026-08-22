//
// Created by TsCat on 2026/7/9.
//
#include "app/service/overlay/tabs/debug_tab.h"

#include <windows.h>
#include <algorithm>
#include <cctype>
#include <memory>
#include <utility>

#include <imgui.h>

#include "app/service/overlay/ui/components.h"
#include "isaac_spy/hook_manager.h"
#include "isaac_spy/isaac/collectible.h"
#include "isaac_spy/isaac/manager.h"
#include "isaac_spy/scanner.h"

namespace isaac = isaac_spy::isaac;

using namespace app::overlay::tabs;

namespace ui = app::overlay::ui;

DebugTab::DebugTab(std::string id, std::string display_name)
    : id(std::move(id)), display_name(std::move(display_name)) {}

void DebugTab::render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ui::metrics::kSectionGap, ui::metrics::kPageGap));
    struct ChildStyleGuard {
        bool active;
        ChildStyleGuard() : active(true) {}
        ~ChildStyleGuard() {
            if (active) {
                ImGui::EndChild();
                ImGui::PopStyleVar();
            }
        }
        ChildStyleGuard(const ChildStyleGuard&) = delete;
        ChildStyleGuard& operator=(const ChildStyleGuard&) = delete;
    } guard;

    if (ImGui::BeginChild("conn_root", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_AlwaysUseWindowPadding,
                          ImGuiWindowFlags_NoScrollbar)) {
        auto* player_manager = isaac::Game::get_instance().get_player_manager();
        if (!player_manager) {
            ui::empty_state("当前不在游戏中", "等待进入游戏");
            return;
        }

        ui::section_header("运行状态", "玩家状态展示");
        ui::status_chip("game_state", isaac::Manager::get_instance().is_in_game() ? "正在游戏" : "主菜单中",
                        isaac::Manager::get_instance().is_in_game() ? ui::kSuccess : ui::kWarning);

        int player_num = player_manager->get_player_num();
        if (player_num <= 0) {
            ui::empty_state("还没有玩家", "开始一局游戏后，这里会显示玩家");
            return;
        }

        ImGui::Text("玩家数量：%d", player_num);

        isaac::Player* local_player = nullptr;
        if (player_manager->is_multi_play()) {
            auto* netplay_manager = isaac::Manager::get_instance().get_netplay_manager();
            if (netplay_manager) {
                local_player = player_manager->find_player(netplay_manager->get_local_player_ptr());
            }
        }
        else {
            local_player = player_manager->get_player(0);
        }

        if (!local_player) {
            ImGui::TextColored(ui::kError, "暂时找不到当前玩家");
            return;
        }

        if (ui::begin_property_table("player_summary")) {
            ui::property_row("最大红心");
            ImGui::Text("%d", local_player->get_max_hearts().get());
            ui::property_row("当前红心");
            ImGui::Text("%d", local_player->get_red_hearts().get());
            ui::property_row("硬币 / 钥匙 / 炸弹");
            ImGui::Text("%d / %d / %d", local_player->get_coins().get(), local_player->get_keys().get(),
                        local_player->get_bombs().get());
            ImGui::EndTable();
        }


        auto coll_vec = local_player->get_collectibles();
        ui::section_header("玩家藏品", "当前玩家收集到的全部藏品。");
        if (ui::begin_data_table("Collectible", 3, ImVec2(0.0f, ui::table_height(6)))) {
            ImGui::TableSetupColumn("藏品", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableSetupColumn("数量", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableHeadersRow();

            for (auto [coll_id, count] : coll_vec) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                bool success = false;
                auto name = isaac::CollectibleDesc(coll_id).get_localized_name(isaac::LANGUAGE_CHINESE, success);
                if (success) {
                    ImGui::Text("%s", name.c_str());
                }
                else {
                    ImGui::Text("未知藏品");
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d", coll_id);
                ImGui::TableNextColumn();
                ImGui::Text("%d", count);
            }
            if (coll_vec.empty()) ui::table_empty_row(3, "当前玩家还没有藏品");
            ui::end_data_table();
        }

        auto pocket_items = local_player->get_pocket_items();
        ui::section_header("道具", "当前玩家携带的卡牌、药丸等道具");
        if (ui::begin_data_table("pocket_items", 3, ImVec2(0.0f, ui::table_height(4)))) {
            ImGui::TableSetupColumn("道具", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableHeadersRow();

            for (auto item : pocket_items) {
                if (item.get_id() == 0) continue;
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool success = false;
                auto name = item.get_localized_name(isaac::LANGUAGE_CHINESE, success);
                if (success) {
                    ImGui::Text("%s", name.c_str());
                }
                else {
                    ImGui::Text("未知道具");
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d", item.get_id());
                ImGui::TableNextColumn();
                ImGui::Text("%d", item.get_type());
            }
            ui::end_data_table();
        }

        auto trinkets = local_player->get_trinkets();
        ui::section_header("饰品", "当前玩家携带的饰品。");
        if (ui::begin_data_table("trinkets", 2, ImVec2(0.0f, ui::table_height(3)))) {
            ImGui::TableSetupColumn("饰品", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableHeadersRow();

            for (auto item : trinkets) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                bool success = false;
                auto name = item.get_localized_name(isaac::LANGUAGE_CHINESE, success);
                if (success) {
                    ImGui::Text("%s", name.c_str());
                }
                else {
                    ImGui::Text("未知饰品");
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d", item.get_id());
            }
            ui::end_data_table();
        }

        static int shake_extent = 20;
        ImGui::InputInt("##extent", &shake_extent, 1, 0);
        ImGui::SameLine(0.0f, 24.0f);
        if (ui::primary_button("震震震")) isaac::Game::get_instance().shake_screen(shake_extent);

        auto configs = isaac::Manager::get_instance().get_item_config_manager();
        if (!configs) return;
        auto all_coll_vec = configs->get_all_collectibles();
        ui::section_header("全部藏品", "按 ID、名字或品质筛选藏品。");
        ui::input_text("debug_collectible_query", "输入 ID 或藏品名字", &collectible_query);
        constexpr const char* qualities[] = {"全部品质", "品质 0", "品质 1", "品质 2", "品质 3", "品质 4"};
        int quality = query_quality + 1;
        if (ui::combo("debug_collectible_quality", &quality, qualities, IM_ARRAYSIZE(qualities))) {
            query_quality = quality - 1;
        }
        if (ui::begin_data_table("All Collectible List", 3,
                                 ImVec2(0.0f, ui::fill_height(0.0f, ui::table_height(8))))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("藏品", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableSetupColumn("品质", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(72.0f));
            ImGui::TableHeadersRow();

            for (auto [coll_id, config] : all_coll_vec) {
                if (query_quality >= 0 && config.quality != query_quality) continue;

                std::string_view key = config.name;
                if (!key.empty() && key[0] == '#') {
                    key.remove_prefix(1);
                }

                bool success = false;
                auto name = isaac::Manager::get_instance().get_string_table()->get_string(
                    "Items", isaac::LANGUAGE_CHINESE, key.data(), success);
                if (!success) name = "未知藏品";
                std::string searchable = name + " " + config.name + " " + std::to_string(coll_id);
                std::string query = collectible_query;
                std::ranges::transform(searchable, searchable.begin(), [](unsigned char c) { return std::tolower(c); });
                std::ranges::transform(query, query.begin(), [](unsigned char c) { return std::tolower(c); });
                if (!query.empty() && !searchable.contains(query)) continue;

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%d", coll_id);
                ImGui::TableNextColumn();
                ImGui::Text("%d", config.quality);
            }
            ui::end_data_table();
        }
    }
}
