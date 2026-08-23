// Created by TsCat on 2026/8/19.
#include "app/service/overlay/tabs/config/config_tab.h"

#include <windows.h>
#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <string>
#include <variant>

#include <imgui.h>

#include "app/event/catalog.h"
#include "app/service/log/log_service.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"

using namespace app::config;

// ConfigTab shell: layout, navigation, save; each page is drawn in its own translation unit:
//   pages/connection.cpp / pages/game.cpp / pages/static_rules.cpp
//   / pages/events.cpp / pages/pickers.cpp / pages/logging.cpp

namespace app::overlay::tabs
{
    namespace detail
    {
        namespace layout
        {
            float kSidebarFooterHeight() { return ui::metrics::scale(72.0f); }
            float kPulseListWidth() { return ui::metrics::scale(230.0f); }
            float kPulseListCompactHeight() { return ui::metrics::scale(184.0f); }
            float kPulseEditorCompactHeight() { return ui::metrics::scale(360.0f); }
            float kPulseTextCompactHeight() { return ui::metrics::scale(220.0f); }
            float kPulseTextHeight() { return ui::metrics::scale(242.0f); }
            float kRuleListWidth() { return ui::metrics::scale(224.0f); }
            float kRuleListCompactHeight() { return ui::metrics::scale(190.0f); }
        }

        const char* const kModifiers[] = {"设为", "增加", "减少", "倍乘"};
        const char* const kScopes[] = {"当前玩家", "其他玩家", "任意玩家", "指定玩家"};

        std::string trim_line(std::string value)
        {
            const auto first = std::ranges::find_if(value, [](const unsigned char c) { return std::isspace(c) == 0; });
            const auto last = std::ranges::find_if(value | std::views::reverse,
                                                   [](const unsigned char c) { return std::isspace(c) == 0; }).base();
            if (first >= last) return {};
            return std::string(first, last);
        }

        bool valid_pulse_frame(const std::string& frame)
        {
            return frame.size() == 16 &&
                   std::ranges::all_of(frame, [](const unsigned char c) { return std::isxdigit(c) != 0; });
        }

        // Draft save gate: structural errors always block saving; messages are the UI hints (Chinese, kept stable verbatim).
        struct ValidationIssue {
            std::string rule_id;
            std::string message;
        };

        void validate_player_filter(const config::PlayerFilterConfig& filter, const std::string& rule_id,
                                    std::vector<ValidationIssue>& issues)
        {
            // Specific scope needs a selected nickname; otherwise the rule is dropped at compile time.
            if (filter.scope == config::PlayerScope::Specific && filter.player_ids.empty())
                issues.push_back({rule_id, "指定玩家需要先选择一名玩家"});
        }

        bool is_pulse_referenced(const config::PulseEffectConfig& pulse, const AppConfig& config,
                                 std::vector<ValidationIssue>& issues, const std::string& rule_id)
        {
            bool valid = true;
            if (!pulse.pulse_a.empty() && !config.pulse_definitions.contains(pulse.pulse_a)) {
                issues.push_back({rule_id, "事件规则引用了不存在的波形,请检查动作里的波形名称"});
                valid = false;
            }
            if (!pulse.pulse_b.empty() && !config.pulse_definitions.contains(pulse.pulse_b)) {
                issues.push_back({rule_id, "事件规则引用了不存在的波形,请检查动作里的波形名称"});
                valid = false;
            }
            return valid;
        }

        std::vector<ValidationIssue> validate_draft(const AppConfig& config)
        {
            std::vector<ValidationIssue> issues;

            for (const auto& [name, frames] : config.pulse_definitions) {
                if (name.empty()) {
                    issues.push_back({"", "波形还没有名字"});
                    continue;
                }
                for (const auto& frame : frames) {
                    if (!valid_pulse_frame(frame)) {
                        issues.push_back({name, "波形“" + name + "”中有格式错误:每帧必须是 16 位十六进制数据"});
                        break;
                    }
                }
            }

            if (config.game.decay.interval.count() < 0 || config.game.climb.interval.count() < 0) {
                issues.push_back({"", "强度变化间隔不能小于 0 毫秒"});
            }

            if (config.game.constant_mode.enabled) {
                const auto& mode = config.game.constant_mode;
                if ((!mode.pulse_a.empty() && !config.pulse_definitions.contains(mode.pulse_a)) ||
                    (!mode.pulse_b.empty() && !config.pulse_definitions.contains(mode.pulse_b))) {
                    issues.push_back({"", "持续模式引用了不存在的波形,请重新选择通道波形"});
                }
            }

            // Static rules
            {
                std::set<std::string> ids;
                for (const auto& rule : config.game.static_rules) {
                    if (rule.rule_id.empty() || !ids.insert(rule.rule_id).second) {
                        issues.push_back({rule.rule_id, "固定规则 ID 不能为空,也不能和其他规则重名"});
                    }
                    validate_player_filter(rule.players, rule.rule_id, issues);
                }
            }

            // Event rules
            {
                std::set<std::string> ids;
                for (const auto& rule : config.game.events) {
                    if (rule.rule_id.empty() || !ids.insert(rule.rule_id).second) {
                        issues.push_back({rule.rule_id, "事件规则 ID 不能为空,也不能和其他规则重名"});
                    }
                    if (event::event_kind_of(rule.event) == event::EventType::None) {
                        issues.push_back({rule.rule_id, "事件规则没有选择触发事件"});
                    }
                    std::visit(
                        [&](const auto& event_config)
                        {
                            using T = std::decay_t<decltype(event_config)>;
                            if constexpr (!std::is_same_v<T, event::NoneEventConfig>)
                                validate_player_filter(event_config.players, rule.rule_id, issues);
                        },
                        rule.event);

                    for (const auto& action : rule.actions) {
                        if (const auto* pulse = std::get_if<config::PulseEffectConfig>(&action)) {
                            if (pulse->duration.count() < 0) {
                                issues.push_back({rule.rule_id, "波形动作的持续时间不能小于 0 毫秒"});
                            }
                            if (pulse->shake_duration.count() < 0) {
                                issues.push_back({rule.rule_id, "波形动作的震动时间不能小于 0 毫秒"});
                            }
                            is_pulse_referenced(*pulse, config, issues, rule.rule_id);
                        }
                        else {
                            const auto& strength = std::get<config::StrengthEffectConfig>(action);
                            if (strength.duration && strength.duration->count() < 0) {
                                issues.push_back({rule.rule_id, "强度动作的持续时间不能小于 0 毫秒"});
                            }
                            if (strength.temporary &&
                                (!strength.duration || strength.duration->count() <= 0)) {
                                issues.push_back({rule.rule_id, "临时强度动作需要正的持续时间"});
                            }
                        }
                    }
                }
            }

            return issues;
        }

        bool draw_pulse_combo(const char* id,
                              const std::unordered_map<std::string, std::vector<std::string>>& pulses,
                              std::string& value)
        {
            std::vector<std::string> names;
            names.emplace_back("(无)");
            for (const auto& [name, frames] : pulses) names.push_back(name);
            std::sort(names.begin() + 1, names.end());
            if (!value.empty() && std::find(names.begin() + 1, names.end(), value) == names.end())
                names.push_back(value);

            int index = value.empty() ? 0 : static_cast<int>(std::find(names.begin(), names.end(), value) - names.begin());
            std::vector<const char*> items;
            items.reserve(names.size());
            for (const auto& n : names) items.push_back(n.c_str());

            const bool changed = app::overlay::ui::combo(id, &index, items.data(), static_cast<int>(items.size()));
            if (changed) value = index == 0 ? "" : names[static_cast<size_t>(index)];
            return changed;
        }
    }

    namespace
    {
        namespace ui = app::overlay::ui;

        constexpr const char* kPages[] = {"概览", "连接与波形", "基础强度", "固定规则", "事件规则", "日志"};
        constexpr const char* kPageDescriptions[] = {
            "预览配置状态",
            "设置 服务器地址 和 波形",
            "基础的强度设置 和 变化机制",
            "按 藏品品质 或 红心差额 换算的强度",
            "让受伤、死亡等事件触发指定波形或强度",
            "日志级别, 输出到控制台和 isaac-coyote.log"
        };

        ImVec4 with_alpha(ImVec4 color, const float alpha)
        {
            color.w = alpha;
            return color;
        }
    }

    ConfigTab::ConfigTab(std::string id, std::string display_name, InputState& input_state,
                         config::ConfigService& config)
        : id_(std::move(id)), display_name_(std::move(display_name)), input_state_(input_state), config_(config) {}

    void ConfigTab::render() {
        if (!state_.initialized) {
            state_.draft = *config_.snapshot();
            state_.initialized = true;
            load_collectibles();
            load_pills();
            load_cards();
        }

        if (const auto shortcut = input_state_.take_captured_shortcut()) {
            state_.draft.overlay.menu_key = *shortcut;
            state_.dirty = true;
        }

        state_.page = std::clamp(state_.page, 0, IM_ARRAYSIZE(kPages) - 1);
        const ImVec2 available = ImGui::GetContentRegionAvail();
        const bool compact = available.x < ui::kCompactWidth;

        const auto draw_current_page = [this]()
        {
            switch (state_.page) {
                case 0: draw_overview();
                    break;
                case 1: draw_connection();
                    break;
                case 2: draw_game();
                    break;
                case 3: draw_static_rules();
                    break;
                case 4: draw_events();
                    break;
                case 5: draw_logging();
                    break;
                default: break;
            }
        };

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::kBackground);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                            ImVec2(ui::metrics::kSectionGap, ui::metrics::kPageGap));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                            ImVec2(ui::metrics::kPageGap, ui::metrics::kPageGap));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                            ImVec2(ui::metrics::kPageGap, ui::metrics::kPageGap));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ui::metrics::kFrameRounding);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, ui::metrics::kFrameRounding);

        if (ImGui::BeginChild("config_root", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_AlwaysUseWindowPadding,
                              ImGuiWindowFlags_NoScrollbar)) {
            if (compact) {
                if (ImGui::BeginChild("page_scroll", ImVec2(0.0f, 0.0f), false,
                                      ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                    if (ui::begin_surface_card("compact_navigation")) {
                        if (ImGui::BeginTable(
                            "title",
                            2,
                            ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoPadOuterX |
                            ImGuiTableFlags_NoPadInnerX)) {
                            ImGui::TableSetupColumn("text", ImGuiTableColumnFlags_WidthStretch);
                            ImGui::TableSetupColumn("button", ImGuiTableColumnFlags_WidthFixed);

                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::AlignTextToFramePadding();
                            ui::title_text("配置");

                            ImGui::TableSetColumnIndex(1);
                            ui::status_chip("status", state_.dirty ? "等待保存" : "已生效",
                                            state_.dirty ? ui::kWarning : ui::kSuccess);
                            ImGui::SameLine();
                            if (ui::primary_button("保存并应用", ImVec2(0.0f, 0.0f))) save_draft();

                            ImGui::EndTable();
                        }
                        ui::gap();

                        ImGui::SetNextItemWidth(-1.0f);
                        ImGui::Combo("##config_page", &state_.page, kPages, IM_ARRAYSIZE(kPages));
                    }
                    ui::end_surface_card();

                    draw_current_page();
                }
                ImGui::EndChild();
            }
            else if (ImGui::BeginTable("config_layout", 3,
                                       ImGuiTableFlags_SizingFixedFit |
                                       ImGuiTableFlags_NoSavedSettings |
                                       ImGuiTableFlags_NoPadOuterX |
                                       ImGuiTableFlags_NoPadInnerX)) {
                ImGui::TableSetupColumn("navigation", ImGuiTableColumnFlags_WidthFixed, ui::kSidebarWidth);
                ImGui::TableSetupColumn("gap", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize,
                                        ui::metrics::kColumnGap);
                ImGui::TableSetupColumn("content", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, ui::metrics::kCardRounding);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                    ImVec2(ui::metrics::kPageGap, ui::metrics::kPageGap));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::kSurface);
                ImGui::PushStyleColor(ImGuiCol_Border, with_alpha(ui::kTextMuted, 0.20f));
                if (ImGui::BeginChild("sidebar", ImVec2(0.0f, 0.0f),
                                      ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                                      ImGuiWindowFlags_NoScrollbar)) {
                    ImGui::Indent(ui::metrics::kPageGap);
                    ui::gap();
                    ImGui::BeginGroup();
                    ui::title_text("配置");
                    ui::muted_text("Isaac X Coyote");
                    ImGui::EndGroup();
                    ui::gap();
                    ImGui::Unindent(ui::metrics::kPageGap);

                    constexpr const char* kNavHints[] = {
                        "状态与概览", "连接与波形", "基础规则", "强度变化", "事件与动作", "日志级别"
                    };
                    const int badges[] = {
                        -1,
                        static_cast<int>(state_.draft.pulse_definitions.size()),
                        -1,
                        static_cast<int>(state_.draft.game.static_rules.size()),
                        static_cast<int>(state_.draft.game.events.size()),
                        -1
                    };
                    static_assert(IM_ARRAYSIZE(kNavHints) == IM_ARRAYSIZE(kPages));
                    static_assert(IM_ARRAYSIZE(badges) == IM_ARRAYSIZE(kPages));
                    for (int i = 0; i < IM_ARRAYSIZE(kPages); ++i) {
                        const std::string item_id = std::to_string(i);
                        if (ui::navigation_item(item_id.c_str(), kPages[i], kNavHints[i], state_.page == i, badges[i]))
                            state_.page = i;
                        ui::gap(ui::metrics::kSmallGap);
                    }

                    const float footer_height = detail::layout::kSidebarFooterHeight();
                    const float remaining = ImGui::GetContentRegionAvail().y;
                    if (remaining > footer_height) ImGui::Dummy(ImVec2(0.0f, remaining - footer_height));
                    ImGui::Separator();
                    ui::gap();
                    ui::status_chip("sidebar_status", state_.dirty ? "待保存" : "已保存",
                                    state_.dirty ? ui::kWarning : ui::kSuccess);
                    ui::gap(ui::metrics::kSmallGap);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(3);

                ImGui::TableSetColumnIndex(2);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ui::kBackground);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                if (ImGui::BeginChild("content_shell", ImVec2(0.0f, 0.0f),
                                      ImGuiChildFlags_AlwaysUseWindowPadding,
                                      ImGuiWindowFlags_NoScrollbar)) {
                    if (ui::page_header(kPages[state_.page], kPageDescriptions[state_.page],
                                        state_.dirty ? "等待保存" : "已应用",
                                        state_.dirty ? ui::kWarning : ui::kSuccess, "保存并应用", state_.dirty))
                        save_draft();
                    if (ImGui::BeginChild("state_.pagescroll", ImVec2(0.0f, 0.0f), false,
                                          ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                        draw_current_page();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                ImGui::EndTable();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(5);
        ImGui::PopStyleColor();

        draw_delete_modal();
        state_.toast.draw();
    }

    void ConfigTab::draw_overview() {
        const float width = ImGui::GetContentRegionAvail().x;
        const int columns = width >= 820.0f ? 4 : (width >= 440.0f ? 2 : 1);

        if (ImGui::BeginTable("overview_summary", columns, ImGuiTableFlags_SizingStretchSame)) {
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 6.0f));

            ImGui::TableNextColumn();
            ui::metric_card("metric_pulses", "波形", std::to_string(state_.draft.pulse_definitions.size()), "来点刺激的?");
            ImGui::TableNextColumn();
            ui::metric_card("metric_static", "固定规则", std::to_string(state_.draft.game.static_rules.size()), "按游戏状态换算强度");
            ImGui::TableNextColumn();
            ui::metric_card("metric_events", "事件规则", std::to_string(state_.draft.game.events.size()), "响应游戏事件");

            std::ostringstream limits;
            limits << "A " << state_.draft.game.strength.limit_a << " / B " << state_.draft.game.strength.limit_b;
            ImGui::TableNextColumn();
            ui::metric_card("metric_limits", "强度上限", limits.str(), "通道 A / B");

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }

        ImGui::Dummy(ImVec2(0.0f, 6.0f));
        if (ui::begin_surface_card("overview_runtime")) {
            ui::section_header("当前配置", "保存后, 新配置将立即生效");
            ImGui::Dummy(ImVec2(0.0f, 4.0f));

            if (ui::begin_property_table("overview_info")) {
                ui::property_row("配置版本");
                ImGui::TextUnformatted(state_.draft.version.empty() ? "Unknown" : state_.draft.version.c_str());

                ui::property_row("服务器地址");
                ImGui::TextWrapped("%s", state_.draft.ws_endpoint.empty() ? "Unknown" : state_.draft.ws_endpoint.c_str());

                ui::property_row("配置状态");
                ui::status_chip("overview_state", state_.dirty ? "等待保存" : "一切就绪",
                                state_.dirty ? ui::kWarning : ui::kSuccess);

                ui::property_row("基础强度");
                ImGui::Text("A %.3f / B %.3f", state_.draft.game.strength.base_a, state_.draft.game.strength.base_b);

                ui::property_row("强度上限");
                ImGui::Text("A %.3f / B %.3f", state_.draft.game.strength.limit_a, state_.draft.game.strength.limit_b);
                ImGui::EndTable();
            }
        }
        ui::end_surface_card();

        ImGui::Dummy(ImVec2(0.0f, ui::kPageGap));
        if (ui::begin_surface_card("overlay_scale")) {
            ui::section_header("界面缩放", "默认为自动, 不舒服可以调节");
            if (ui::begin_property_table("overlay_scale_properties")) {
                ui::property_row("自动调整");
                state_.dirty |= ImGui::Checkbox("##automatic_dpi", &state_.draft.overlay.automatic_dpi);
                const float effective_scale = state_.draft.overlay.automatic_dpi
                                                  ? ui::metrics::current_scale()
                                                  : state_.draft.overlay.scale;
                ui::property_row("当前 DPI");
                ImGui::Text("%.0f%%(DPI %u)",
                            effective_scale * 100.0f,
                            static_cast<UINT>(ui::metrics::current_scale() * 96.0f));
                ImGui::BeginDisabled(state_.draft.overlay.automatic_dpi);
                ui::property_row("手动调整");
                ImGui::SetNextItemWidth(-1.0f);
                float scale_percent = state_.draft.overlay.scale * 100.0f;
                if (ImGui::SliderFloat("##overlay_scale_value", &scale_percent, 75.0f, 250.0f, "%.0f%%",
                                       ImGuiSliderFlags_AlwaysClamp)) {
                    state_.draft.overlay.scale = std::round(scale_percent / 5.0f) * 0.05f;
                    state_.dirty = true;
                }
                ImGui::EndDisabled();

                ui::property_row("菜单快捷键");
                ImGui::Text("VK 0x%02X", state_.draft.overlay.menu_key);
                ImGui::SameLine();
                if (input_state_.is_capturing_shortcut()) {
                    ImGui::TextDisabled("请按下新按键...");
                    ImGui::SameLine();
                    if (ui::tonal_button("取消")) input_state_.cancel_shortcut_capture();
                }
                else if (ui::tonal_button("重新绑定")) {
                    input_state_.begin_shortcut_capture();
                }
                ImGui::EndTable();
            }
        }
        ui::end_surface_card();
    }

    void ConfigTab::save_draft() {
        const auto issues = detail::validate_draft(state_.draft);
        if (!issues.empty()) {
            state_.status = issues.front().message;
            state_.toast.notify(state_.status, ui::ToastType::Error);
            return;
        }
        if (!config_.save(state_.draft)) {
            state_.status = "保存失败,未改动当前配置";
            state_.toast.notify(state_.status, ui::ToastType::Error);
            return;
        }

        config_.publish(state_.draft);
        app::log::apply_config(state_.draft.logging);
        state_.dirty = false;
        state_.status = "保存完成,新配置已生效";
        state_.toast.notify(state_.status);
    }

    void ConfigTab::draw_delete_modal() {
        const bool open = state_.pending_delete != DeleteTarget::None;
        const auto result = ui::confirm_modal("确认删除##config", open, "真的要删掉它吗?", "保存后才会正式生效", "确认删除");
        if (result == ui::ConfirmResult::None) return;
        if (result == ui::ConfirmResult::Confirmed) {
            switch (state_.pending_delete) {
                case DeleteTarget::Pulse:
                    state_.draft.pulse_definitions.erase(state_.pending_delete_name);
                    break;
                case DeleteTarget::StaticRule:
                    if (state_.pending_delete_index >= 0 && state_.pending_delete_index < static_cast<int>(state_.draft.
                        game.static_rules.size()))
                        state_.draft.game.static_rules.erase(
                            state_.draft.game.static_rules.begin() + state_.pending_delete_index);
                    state_.selected_static = std::min(state_.selected_static,
                                                      static_cast<int>(state_.draft.game.static_rules.size()) - 1);
                    break;
                case DeleteTarget::EventRule:
                    if (state_.pending_delete_index >= 0 && state_.pending_delete_index < static_cast<int>(state_.draft.
                        game.events.size()))
                        state_.draft.game.events.erase(state_.draft.game.events.begin() + state_.pending_delete_index);
                    state_.selected_event = std::min(state_.selected_event,
                                                      static_cast<int>(state_.draft.game.events.size()) - 1);
                    break;
                case DeleteTarget::EventAction:
                    if (state_.selected_event >= 0 && state_.selected_event < static_cast<int>(state_.draft.game.events.
                            size()) &&
                        state_.pending_delete_index >= 0 && state_.pending_delete_index < static_cast<int>(state_.draft.
                            game.events[state_.selected_event].actions.size()))
                        state_.draft.game.events[state_.selected_event].actions.erase(
                            state_.draft.game.events[state_.selected_event].actions.begin() + state_.
                            pending_delete_index);
                    break;
                case DeleteTarget::CollectibleOverride:
                    if (state_.selected_static >= 0 && state_.selected_static < static_cast<int>(state_.draft.game.
                        static_rules.size())) {
                        if (auto* source = std::get_if<CollectibleSourceConfig>(
                            &state_.draft.game.static_rules[state_.selected_static].source))
                            source->override_rule.erase(state_.pending_collectible_id);
                    }
                    break;
                case DeleteTarget::None: break;
            }
            state_.dirty = true;
        }
        state_.pending_delete = DeleteTarget::None;
        state_.pending_delete_index = -1;
        state_.pending_collectible_id = 0;
        state_.pending_delete_name.clear();
    }
}