#include "app/service/overlay/tabs/statistics_tab.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include <imgui.h>

#include "app/service/overlay/ui/components.h"
#include "app/service/overlay/ui/themes.h"

namespace app::overlay::tabs
{
    namespace
    {
        constexpr ImVec4 kChannelA{0.961f, 0.620f, 0.043f, 1.0f};
        constexpr ImVec4 kChannelB{0.220f, 0.741f, 0.957f, 1.0f};
        constexpr ImVec4 kTrackBackground{1.0f, 1.0f, 1.0f, 0.08f};

        const char* operation_name(const std::optional<rule::Modifier>& modifier) {
            if (!modifier) return "未修改";
            switch (modifier->operation) {
                case rule::ModifierOperation::Override: return "设为";
                case rule::ModifierOperation::Increase: return "+";
                case rule::ModifierOperation::Decrease: return "-";
                case rule::ModifierOperation::Multiply: return "×";
            }
            return "未知";
        }

        std::string modifier_value(const std::optional<rule::Modifier>& modifier) {
            if (!modifier) return "";

            char text[64]{};
            std::snprintf(text, sizeof(text), "%s %.0f", operation_name(modifier), modifier->value);
            return text;
        }

        bool has_change(const rule::RuleExplanation& explanation) {
            for (const auto& step : explanation.steps) {
                if (std::abs(step.after_a - step.before_a) >= 0.0001 ||
                    std::abs(step.after_b - step.before_b) >= 0.0001)
                    return true;
            }
            return false;
        }

        std::pair<double, double> explanation_delta(const rule::RuleExplanation& explanation) {
            double delta_a = 0.0;
            double delta_b = 0.0;
            for (const auto& step : explanation.steps) {
                delta_a += step.after_a - step.before_a;
                delta_b += step.after_b - step.before_b;
            }
            return {delta_a, delta_b};
        }

        const rule::RuleContributionSummary* contribution_for(
            const rule::StateSnapshot& snapshot, const rule::RuleId& rule_id) {
            for (const auto& contribution : snapshot.contributions) {
                if (contribution.rule_id == rule_id)
                    return &contribution;
            }
            return nullptr;
        }

        void draw_channel_track(const char* label, const double output, const double target,
                                const double limit, const double base, const double unclamped,
                                const ImVec4& color) {
            const double safe_limit = std::max(limit, 1.0);
            const auto ratio = [](const double value, const double maximum)
            {
                return static_cast<float>(std::clamp(value / maximum, 0.0, 1.0));
            };

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(label);
            ImGui::PopStyleColor();
            ImGui::SameLine(0.0f, ui::metrics::scale(10.0f));
            ImGui::SetWindowFontScale(1.28f);
            ImGui::Text("%.0f", output);
            ImGui::SetWindowFontScale(1.0f);
            ImGui::SameLine(0.0f, ui::metrics::scale(8.0f));
            ImGui::TextDisabled("目标 %.0f / %.0f", target, limit);

            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(5.0f)));
            const ImVec2 start = ImGui::GetCursorScreenPos();
            const float width = std::max(ImGui::GetContentRegionAvail().x, ui::metrics::scale(80.0f));
            const float height = ui::metrics::scale(9.0f);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            draw_list->AddRectFilled(start, ImVec2(start.x + width, start.y + height),
                                     ImGui::GetColorU32(kTrackBackground), height * 0.5f);
            draw_list->AddRectFilled(
                start,
                ImVec2(start.x + width * ratio(target, safe_limit), start.y + height),
                ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, 0.78f)), height * 0.5f);

            const float base_x = start.x + width * ratio(base, safe_limit);
            const float unclamped_x = start.x + width * ratio(unclamped, safe_limit);
            const float output_x = start.x + width * ratio(output, safe_limit);
            draw_list->AddLine(ImVec2(base_x, start.y - ui::metrics::scale(3.0f)),
                               ImVec2(base_x, start.y + height + ui::metrics::scale(3.0f)),
                               ImGui::GetColorU32(ui::kTextMuted), ui::metrics::scale(1.0f));
            draw_list->AddLine(ImVec2(unclamped_x, start.y - ui::metrics::scale(3.0f)),
                               ImVec2(unclamped_x, start.y + height + ui::metrics::scale(3.0f)),
                               ImGui::GetColorU32(ui::kWarning), ui::metrics::scale(1.0f));
            draw_list->AddLine(ImVec2(output_x, start.y - ui::metrics::scale(4.0f)),
                               ImVec2(output_x, start.y + height + ui::metrics::scale(4.0f)),
                               ImGui::GetColorU32(ui::kText), ui::metrics::scale(2.0f));
            ImGui::Dummy(ImVec2(width, height + ui::metrics::scale(5.0f)));

            ImGui::TextDisabled("基础 %.0f", base);
            ImGui::SameLine(0.0f, ui::metrics::scale(12.0f));
            ImGui::TextColored(ui::kWarning, "规则后 %.0f", unclamped);
            ImGui::SameLine(0.0f, ui::metrics::scale(12.0f));
            ImGui::TextDisabled("实际 %.0f", output);
        }

        void draw_step(const rule::ExplanationStep& step) {
            const auto delta_a = step.after_a - step.before_a;
            const auto delta_b = step.after_b - step.before_b;
            if (std::abs(delta_a) < 0.0001 && std::abs(delta_b) < 0.0001)
                return;

            if (ui::begin_data_table("modifier_step", 3, ImVec2(0.0f, ui::table_height(2, false)),
                                     ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV)) {
                ImGui::TableSetupColumn("channel", ImGuiTableColumnFlags_WidthFixed,
                                        ui::metrics::scale(56.0f));
                ImGui::TableSetupColumn("change", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("delta", ImGuiTableColumnFlags_WidthFixed,
                                        ui::metrics::scale(76.0f));

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(kChannelA, "A");
                ImGui::TableNextColumn();
                ImGui::Text("%.0f %s = %.0f", step.before_a, modifier_value(step.a_modifier).c_str(),
                            step.after_a);
                ImGui::TableNextColumn();
                ImGui::TextColored(delta_a >= 0.0 ? ui::kWarning : ui::kSuccess, "%+.0f", delta_a);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::TextColored(kChannelB, "B");
                ImGui::TableNextColumn();
                ImGui::Text("%.0f %s = %.0f", step.before_b, modifier_value(step.b_modifier).c_str(),
                            step.after_b);
                ImGui::TableNextColumn();
                ImGui::TextColored(delta_b >= 0.0 ? ui::kWarning : ui::kSuccess, "%+.0f", delta_b);

                ui::end_data_table();
            }

            if (step.remaining_seconds)
                ImGui::TextDisabled("临时 - 剩余 %d 秒", *step.remaining_seconds);
        }

        void draw_collectible_details(const std::vector<rule::CollectibleContributionDetail>& details) {
            if (details.empty()) return;

            auto draw_modifier_cell = [](const std::optional<rule::Modifier>& modifier) {
                if (!modifier) {
                    ImGui::TextDisabled("-");
                    return;
                }
                ImGui::TextColored(modifier->value >= 0.0 ? ui::kWarning : ui::kSuccess, "%s",
                                   modifier_value(modifier).c_str());
            };

            ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
            ImGui::TextDisabled("藏品明细");
            if (!ui::begin_data_table(
                "collectible_details", 6,
                ImVec2(0.0f, ui::table_height(4))))
                return;

            ImGui::TableSetupColumn("藏品", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("品质", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(48.0f));
            ImGui::TableSetupColumn("数量", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(48.0f));
            ImGui::TableSetupColumn("规则", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(76.0f));
            ImGui::TableSetupColumn("A", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(68.0f));
            ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_WidthFixed, ui::metrics::scale(68.0f));
            ImGui::TableHeadersRow();

            for (const auto& detail : details) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const std::string name = (detail.name.empty() ? "未知藏品" : detail.name) +
                        "  #" + std::to_string(detail.id);
                if (detail.matched)
                    ImGui::TextUnformatted(name.c_str());
                else
                    ImGui::TextDisabled("%s", name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%d", detail.quality);
                ImGui::TableNextColumn();
                ImGui::Text("%d", detail.count);
                ImGui::TableNextColumn();
                if (detail.matched)
                    ImGui::TextColored(ui::kSuccess, "%s", detail.rule_source.c_str());
                else
                    ImGui::TextDisabled("%s", detail.rule_source.c_str());
                ImGui::TableNextColumn();
                draw_modifier_cell(detail.modifiers.a);
                ImGui::TableNextColumn();
                draw_modifier_cell(detail.modifiers.b);
            }

            ui::end_data_table();
        }

        void draw_path_stage(const char* label, const char* hint, const double a, const double b,
                             const ImVec4& color) {
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(label);
            ImGui::PopStyleColor();
            ImGui::TextDisabled("%s", hint);
            ImGui::Text("A %.0f  ·  B %.0f", a, b);
        }
    }

    void StatisticsTab::render() {
        const auto snapshot = game_.state_hub().snapshot();
        if (snapshot->created_at == std::chrono::steady_clock::time_point{}) {
            ui::empty_state("暂无强度统计", "触发规则后，这里会显示统计");
            return;
        }

        const bool clamped = snapshot->target != snapshot->unclamped_target;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ui::metrics::kSectionGap, ui::metrics::kPageGap));
        if (ImGui::BeginChild("statistics_root", ImVec2(0.0f, 0.0f),
                              ImGuiChildFlags_AlwaysUseWindowPadding,
                              ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            if (ui::begin_surface_card("strength_overview")) {
                ui::section_header("强度轨迹", "从基础值到实际输出，按规则顺序查看 A/B 的变化");

                if (snapshot->reset_pending) {
                    ImGui::TextColored(ui::kWarning, "运行状态正在重置，当前轨迹可能暂时不完整");
                }

                if (ui::begin_columns("strength_channels", ui::metrics::scale(280.0f), 2,
                                      ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV)) {
                    ImGui::TableNextColumn();
                    draw_channel_track("A", snapshot->output.a, snapshot->target.a, snapshot->limits.a,
                                       snapshot->base.a, snapshot->unclamped_target.a, kChannelA);
                    ImGui::TableNextColumn();
                    draw_channel_track("B", snapshot->output.b, snapshot->target.b, snapshot->limits.b,
                                       snapshot->base.b, snapshot->unclamped_target.b, kChannelB);
                    ui::end_columns();
                }

                if (clamped) {
                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
                    ImGui::TextColored(ui::kError, "已触发通道上限 · A %.0f / B %.0f",
                                       snapshot->limits.a, snapshot->limits.b);
                }
            }
            ui::end_surface_card();

            if (ui::begin_surface_card("calculation_path")) {
                ui::section_header("计算路径", "分段处理步骤");
                if (ui::begin_columns("path_stages", ui::metrics::scale(150.0f), 4,
                                      ImGuiTableFlags_NoSavedSettings)) {
                    ImGui::TableNextColumn();
                    draw_path_stage("BASE", "初始强度", snapshot->base.a, snapshot->base.b, ui::kTextMuted);
                    ImGui::TableNextColumn();
                    draw_path_stage("RULE", "规则", snapshot->unclamped_target.a,
                                    snapshot->unclamped_target.b, kChannelA);
                    ImGui::TableNextColumn();
                    draw_path_stage("CLAMP", clamped ? "已限幅" : "未限幅", snapshot->target.a,
                                    snapshot->target.b, clamped ? ui::kError : ui::kSuccess);
                    ImGui::TableNextColumn();
                    draw_path_stage("OUTPUT", "实际输出", snapshot->output.a, snapshot->output.b, ui::kText);
                    ui::end_columns();
                }
            }
            ui::end_surface_card();

            ui::section_header("规则强度", "展开看看计算细节和藏品明细");
            bool rendered_explanation = false;
            for (std::size_t index = 0; index < snapshot->explanations.size(); ++index) {
                const auto& explanation = snapshot->explanations[index];
                const auto* contribution = contribution_for(*snapshot, explanation.rule_id);

                const bool stale = contribution != nullptr && contribution->stale;
                if (!has_change(explanation) && explanation.collectible_details.empty() && !stale)
                    continue;

                rendered_explanation = true;
                const auto [delta_a, delta_b] = explanation_delta(explanation);
                const char* kind = explanation.kind == rule::ExplanationKind::Event ? "EVENT" : "STATIC";
                const std::string title = explanation.name.empty() ? explanation.rule_id : explanation.name;
                const std::string id = "rule_explanation_" + std::to_string(index);

                if (ui::begin_surface_card(id.c_str())) {
                    if (ImGui::BeginTable("rule_head", 5, ImGuiTableFlags_SizingFixedFit| ImGuiTableFlags_NoPadOuterX)) {
                        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("chip", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("spacer",ImGuiTableColumnFlags_WidthStretch);
                        ImGui::TableSetupColumn("preview_a", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableSetupColumn("preview_b", ImGuiTableColumnFlags_WidthFixed);
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::PushStyleColor(ImGuiCol_Text, stale ? ui::kWarning : ui::kText);
                        ImGui::TextUnformatted(title.c_str());
                        ImGui::PopStyleColor();

                        ImGui::TableSetColumnIndex(1);
                        ui::status_chip((id + "_kind").c_str(), kind, stale ? ui::kWarning : ui::kAccent);

                        ImGui::TableSetColumnIndex(2);

                        ImGui::TableSetColumnIndex(3);
                        ImGui::TextColored(delta_a >= 0.0 ? ui::kWarning : ui::kSuccess, "A %+.0f", delta_a);

                        ImGui::TableSetColumnIndex(4);
                        ImGui::TextColored(delta_b >= 0.0 ? ui::kWarning : ui::kSuccess, "B %+.0f", delta_b);
                    }
                    ImGui::EndTable();

                    if (stale) {
                        const char* reason = contribution->stale_reason
                                                 ? contribution->stale_reason->c_str()
                                                 : "当前数据不可用";
                        ImGui::TextColored(ui::kWarning, "暂时使用缓存 - %s", reason);
                    }

                    ImGui::Dummy(ImVec2(0.0f, ui::metrics::scale(4.0f)));
                    for (const auto& step : explanation.steps)
                        draw_step(step);
                    draw_collectible_details(explanation.collectible_details);
                }
                ui::end_surface_card();
            }

            if (!rendered_explanation)
                ui::empty_state("无规则生效", "触发规则后会显示详情");
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
}
