// Created by TsCat on 2026/8/19.
#include <format>
#include <ranges>
#include <string_view>

#include <imgui.h>
#include <imgui_stdlib.h>

#include "app/service/overlay/tabs/config/config_tab.h"
#include "app/service/overlay/tabs/config/config_tab_internal.h"
#include "app/service/overlay/ui/themes.h"

// 连接与波形页:服务器地址 + 波形定义编辑。

namespace app::overlay::tabs
{
    namespace
    {
        namespace ui = app::overlay::ui;
    }

    void ConfigTab::draw_connection() {
        if (ui::begin_surface_card("connection_service")) {
            ui::section_header("服务器地址", "请填写 DGLAB V4 WebSocket 服务器地址");
            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

            if (ui::begin_property_table("connection_properties")) {
                ui::property_row("地址");
                if (ui::input_text("ws_endpoint", "wss://example.com/v4", &state_.draft.ws_endpoint)) {
                    state_.dirty = true;
                }
                ImGui::EndTable();
            }
        }
        ui::end_surface_card();
        ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(12.0f)));

        auto& pulses = state_.draft.pulse_definitions;
        auto& selected_pulse = state_.selected_pulse;

        const auto make_unique_pulse_name = [&]() {
            for (std::size_t index = 1;; ++index) {
                std::string candidate = std::format("未命名波形 {}", index);
                if (!pulses.contains(candidate)) return candidate;
            }
        };

        const auto load_pulse_editor = [&](std::string_view name) {
            state_.pulse_editor_key.assign(name);
            state_.pulse_name_draft.assign(name);
            state_.pulse_name_error.clear();
            state_.pulse_frames_draft.clear();

            const auto it = pulses.find(state_.pulse_editor_key);
            if (it == pulses.end()) return;

            std::size_t required_capacity = 0;
            for (const auto& frame : it->second) required_capacity += frame.size() + 1;

            state_.pulse_frames_draft.reserve(required_capacity);
            for (const auto& frame : it->second) {
                state_.pulse_frames_draft.append(frame);
                state_.pulse_frames_draft.push_back('\n');
            }
        };

        const auto select_pulse = [&](std::string_view name) {
            selected_pulse.assign(name);
            load_pulse_editor(selected_pulse);
        };

        const auto parse_frame_text = [&](auto& frames) {
            frames.clear();
            const std::string& text = state_.pulse_frames_draft;
            std::size_t line_begin = 0;

            while (line_begin < text.size()) {
                const std::size_t line_end = text.find('\n', line_begin);
                std::string line = text.substr(line_begin, line_end == std::string::npos ? std::string::npos : line_end - line_begin);
                line = detail::trim_line(std::move(line));
                if (!line.empty()) frames.push_back(std::move(line));
                if (line_end == std::string::npos) break;
                line_begin = line_end + 1;
            }
        };

        if (!selected_pulse.empty() && !pulses.contains(selected_pulse)) selected_pulse.clear();
        if (selected_pulse.empty() && !pulses.empty()) selected_pulse = pulses.begin()->first;
        if (state_.pulse_editor_key != selected_pulse) load_pulse_editor(selected_pulse);

        const bool compact = ui::is_compact(ui::metrics::kTwoColumnWidth);
        const int column_count = compact ? 1 : 2;
        constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings;
        const ImGuiTableFlags effective_flags = table_flags | (compact ? ImGuiTableFlags_None : ImGuiTableFlags_BordersInnerV);

        if (!ImGui::BeginTable("pulse_detail", column_count, effective_flags)) return;

        if (!compact) {
            ImGui::TableSetupColumn("pulse_list", ImGuiTableColumnFlags_WidthFixed, detail::layout::kPulseListWidth());
            ImGui::TableSetupColumn("pulse_editor", ImGuiTableColumnFlags_WidthStretch);
        }

        ImGui::TableNextColumn();
        const float list_height = compact ? detail::layout::kPulseListCompactHeight() : ImGui::GetContentRegionAvail().y;

        if (ui::begin_surface_card("pulse_list_panel", ImVec2(0.0f, list_height), false)) {
            ui::title_text("波形");
            ImGui::TextColored(ui::kTextMuted, "共 %zu 个", pulses.size());

            if (ui::primary_button("新建", ImVec2(ui::metrics::scale(104.0f), ui::metrics::scale(34.0f)))) {
                const std::string new_name = make_unique_pulse_name();
                const auto [it, inserted] = pulses.try_emplace(new_name);
                if (inserted) { select_pulse(it->first); state_.dirty = true; }
            }

            if (!selected_pulse.empty()) {
                ImGui::SameLine();
                if (ui::danger_button("删除", ImVec2(ui::metrics::scale(72.0f), ui::metrics::scale(34.0f)))) {
                    state_.pending_delete = DeleteTarget::Pulse;
                    state_.pending_delete_name = selected_pulse;
                }
            }

            ImGui::Separator();
            for (const auto& [name, frames] : pulses) {
                const std::string hint = std::format("{} 帧", frames.size());
                if (ui::list_navigation_item(name.c_str(), name.c_str(), hint.c_str(), selected_pulse == name, "")) {
                    select_pulse(name);
                }
                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(3.0f)));
            }
        }
        ui::end_surface_card();

        ImGui::TableNextColumn();
        if (compact) ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));

        const float editor_height = compact ? detail::layout::kPulseEditorCompactHeight() : ImGui::GetContentRegionAvail().y;

        if (ui::begin_surface_card("pulse_editor", ImVec2(0.0f, editor_height), false)) {
            if (selected_pulse.empty() || !pulses.contains(selected_pulse)) {
                ui::empty_state("这里还是空的", "先创建一段波形吧");
            } else {
                const auto& current_frames = pulses.at(selected_pulse);
                const auto invalid_count = std::ranges::count_if(current_frames, [](const std::string& frame) {
                    return !detail::valid_pulse_frame(frame);
                });

                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
                ui::title_text(selected_pulse.c_str());
                ImGui::SameLine();
                ui::status_chip("pulse_validation", invalid_count > 0 ? "波形无效" : "检查通过",
                    invalid_count > 0 ? ui::kError : ui::kSuccess);
                ui::muted_text(std::format("共 {} 帧", current_frames.size()).c_str());
                if (invalid_count > 0) {
                    ImGui::SameLine(0, ui::metrics::scale(12.0f));
                    ImGui::TextColored(ui::kError,"%td 个无效帧", static_cast<std::ptrdiff_t>(invalid_count));
                }

                if (ui::input_text("pulse_name", "名称", &state_.pulse_name_draft)) {
                    state_.pulse_name_error.clear();
                }

                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    std::string new_name = detail::trim_line(state_.pulse_name_draft);
                    if (new_name.empty()) {
                        state_.pulse_name_error = "波形名称不能为空";
                    } else if (new_name != selected_pulse && pulses.contains(new_name)) {
                        state_.pulse_name_error = "已经存在同名波形";
                    } else if (new_name != selected_pulse) {
                        auto node = pulses.extract(selected_pulse);
                        if (!node.empty()) {
                            node.key() = new_name;
                            pulses.insert(std::move(node));
                            selected_pulse = new_name;
                            state_.pulse_editor_key = new_name;
                            state_.pulse_name_draft = new_name;
                            state_.dirty = true;
                        }
                    } else {
                        state_.pulse_name_draft = selected_pulse;
                    }
                }

                if (!state_.pulse_name_error.empty()) {
                    ImGui::TextColored(ui::kError, "%s", state_.pulse_name_error.c_str());
                }

                ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(6.0f)));

                auto& frames = pulses.at(selected_pulse);
                const float text_height = compact ? detail::layout::kPulseTextCompactHeight() : detail::layout::kPulseTextHeight();

                if (ui::input_multiline("pulse_frames", &state_.pulse_frames_draft, ImVec2(-1.0f, text_height))) {
                    parse_frame_text(frames);
                    state_.dirty = true;
                }
            }
        }

        ui::end_surface_card();
        ImGui::EndTable();
    }
}