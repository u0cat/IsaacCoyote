//
// Created by TsCat on 2026/7/7.
//

#include "app/service/overlay/coyote_monitor.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "app/service/overlay/ui/themes.h"

using namespace app::overlay;

namespace
{
    constexpr std::chrono::seconds kPulseHistory{2};
    constexpr float kPulseStrengthMaximum = 100.0f;
    constexpr float kMonitorAspect = 560.0f / 210.0f;
    constexpr float kMinimumMonitorWidth = 260.0f;

    ImU32 color_with_alpha(const ImVec4& color, const float alpha) {
        return ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, alpha));
    }

    ImU32 monitor_color(const float alpha = 1.0f) {
        return ImGui::GetColorU32(ImVec4(1.0f, 0.91f, 0.58f, alpha));
    }

    void constrain_monitor_size(ImGuiSizeCallbackData* data) {
        data->DesiredSize.x = std::max(data->DesiredSize.x, ui::metrics::scale(kMinimumMonitorWidth));
        data->DesiredSize.y = data->DesiredSize.x / kMonitorAspect;
    }
}

void CoyoteMonitor::render(const bool interactive) {
    const auto snapshot = game_.state_hub().snapshot();
    if (snapshot->created_at == std::chrono::steady_clock::time_point{} || snapshot->reset_pending)
        return;

    const int strength_a = static_cast<int>(std::lround(snapshot->output.a));
    const int strength_b = static_cast<int>(std::lround(snapshot->output.b));
    const int limit_a = static_cast<int>(std::lround(snapshot->limits.a));
    const int limit_b = static_cast<int>(std::lround(snapshot->limits.b));
    const auto pulse_snapshot = game_.pulse_monitor_snapshot();
    const auto now = std::chrono::steady_clock::now();

    ImGui::SetNextWindowSize(ui::metrics::scale(ImVec2(560.0f, 210.0f)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(
        ui::metrics::scale(ImVec2(kMinimumMonitorWidth, kMinimumMonitorWidth / kMonitorAspect)),
        ImVec2(FLT_MAX, FLT_MAX), constrain_monitor_size);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar;
    if (!interactive) flags |= ImGuiWindowFlags_NoInputs;
    ImGui::Begin("CoyoteMonitorV2", nullptr, flags);

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float monitor_scale = avail.x / ui::metrics::scale(544.0f);
    const float spacing = ui::metrics::scale(8.0f) * monitor_scale;
    const float channel_width = (avail.x - spacing) * 0.5f;
    const float channel_height = avail.y;

    ImGui::BeginGroup();
    draw_channel("A", pulse_snapshot->channel_a, strength_a, limit_a,
                 ImVec2(channel_width, channel_height), now);
    ImGui::SameLine(0, spacing);
    draw_channel("B", pulse_snapshot->channel_b, strength_b, limit_b,
                 ImVec2(channel_width, channel_height), now);
    ImGui::EndGroup();

    ImGui::End();
}

void CoyoteMonitor::draw_channel(const char* id,
                                 const std::vector<coyote::PulseMonitorSample>& samples,
                                 const int current_value,
                                 const int limit,
                                 const ImVec2 size,
                                 const std::chrono::steady_clock::time_point now) {
    ImGui::PushID(id);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const float scale = std::max(size.x / ui::metrics::scale(268.0f), 0.1f);
    const auto px = [scale](const float value) {
        return ui::metrics::scale(value) * scale;
    };

    const float rounding = px(8.0f);
    const float outline_width = px(2.0f);
    const float wave_height = size.y * 0.46f;
    const ImVec2 wave_min = origin;
    const ImVec2 wave_max(origin.x + size.x, origin.y + wave_height);

    draw_list->AddRectFilled(wave_min, wave_max, color_with_alpha(ui::kSurface, 0.92f), rounding);
    draw_list->AddRect(wave_min, wave_max, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.92f)),
                       rounding, 0, outline_width);

    const float wave_padding = px(8.0f);
    const float inner_width = std::max(size.x - wave_padding * 2.0f, 1.0f);
    const float inner_height = std::max(wave_height - wave_padding * 2.0f, 1.0f);
    const float nominal_stride = inner_width / 80.0f;
    const float bar_width = std::max(px(1.0f), nominal_stride * 0.68f);
    const float history_ms = static_cast<float>(
        std::chrono::duration_cast<std::chrono::milliseconds>(kPulseHistory).count());

    draw_list->PushClipRect(ImVec2(wave_min.x + wave_padding, wave_min.y + wave_padding),
                            ImVec2(wave_max.x - wave_padding, wave_max.y - wave_padding), true);
    for (const auto& sample : samples) {
        const float age_ms = static_cast<float>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now - sample.created_at).count());
        if (age_ms < 0.0f || age_ms > history_ms || sample.strength == 0)
            continue;

        const float timeline_ratio = 1.0f - age_ms / history_ms;
        const float strength_ratio = std::clamp(
            static_cast<float>(sample.strength) / kPulseStrengthMaximum, 0.0f, 1.0f);
        const float x = wave_min.x + wave_padding + timeline_ratio * inner_width;
        const float bottom = wave_max.y - wave_padding;
        const float top = bottom - inner_height * strength_ratio;
        draw_list->AddRectFilled(ImVec2(x - bar_width * 0.5f, top),
                                 ImVec2(x + bar_width * 0.5f, bottom), monitor_color(0.96f));
    }
    draw_list->PopClipRect();

    const float lower_top = wave_max.y + px(10.0f);
    const float lower_height = std::max(origin.y + size.y - lower_top, px(34.0f));
    const float track_width = px(14.0f);
    const float track_height = std::max(lower_height - px(4.0f), px(30.0f));
    const ImVec2 track_min(origin.x + px(14.0f), lower_top);
    const ImVec2 track_max(track_min.x + track_width, track_min.y + track_height);
    const float track_rounding = track_width * 0.25f;

    draw_list->AddRectFilled(track_min, track_max, color_with_alpha(ui::kSurface, 0.9f), track_rounding);
    draw_list->AddRect(track_min, track_max, ImGui::GetColorU32(ImVec4(0, 0, 0, 0.82f)),
                       track_rounding, 0, outline_width);

    const int safe_limit = std::max(limit, 1);
    const float strength_ratio = std::clamp(static_cast<float>(current_value) / safe_limit, 0.0f, 1.0f);
    if (strength_ratio > 0.0f) {
        const float inset = px(3.0f);
        const float fill_bottom = track_max.y - inset;
        const float fill_top = fill_bottom - (track_height - inset * 2.0f) * strength_ratio;
        draw_list->AddRectFilled(ImVec2(track_min.x + inset, fill_top),
                                 ImVec2(track_max.x - inset, fill_bottom), monitor_color(0.92f),
                                 std::max(track_rounding - inset, 0.0f));
    }

    char value_text[32]{};
    char ratio_text[64]{};
    std::snprintf(value_text, sizeof(value_text), "%d", current_value);
    std::snprintf(ratio_text, sizeof(ratio_text), "%d/%d", current_value, limit);

    ImFont* font = ImGui::GetFont();
    const float value_font_size = ImGui::GetFontSize() * 3.0f * scale;
    const float ratio_font_size = ImGui::GetFontSize() * 2.0f * scale;
    const float text_x = track_max.x + px(14.0f);
    const float block_height = value_font_size + px(3.0f) + ratio_font_size;
    const float text_y = lower_top + std::max((track_height - block_height) * 0.5f, 0.0f);
    draw_list->AddText(font, value_font_size, ImVec2(text_x, text_y), monitor_color(), value_text);
    draw_list->AddText(font, ratio_font_size,
                       ImVec2(text_x, text_y + value_font_size + px(3.0f)),
                       color_with_alpha(ui::kText, 0.82f), ratio_text);

    ImGui::Dummy(size);
    ImGui::PopID();
}
