//
// Created by TsCat on 2026/7/28.
//

#ifndef ISAACCOYOTE_THEMES_H
#define ISAACCOYOTE_THEMES_H

#include <imgui.h>

namespace app::overlay::ui
{
    namespace metrics
    {
        inline float& current_scale() {
            static float s = 1.0f;
            return s;
        }

        inline float scale(float px) {
            return px * current_scale();
        }

        inline ImVec2 scale(ImVec2 v) {
            return ImVec2(v.x * current_scale(), v.y * current_scale());
        }

        inline constexpr ImVec2 kDefaultWindowSize{820.0f, 640.0f};
        inline constexpr ImVec2 kMinimumMonitorSize{100.0f, 180.0f};
        inline constexpr float kCompactWidth = 760.0f;
        inline constexpr float kTwoColumnWidth = 650.0f;
        inline constexpr float kSidebarWidth = 208.0f;
        inline constexpr float kLabelWidth = 156.0f;
        inline constexpr float kPageGap = 8.0f;
        inline constexpr float kSmallGap = 4.0f;
        inline constexpr float kSectionGap = 12.0f;
        inline constexpr float kColumnGap = 12.0f;
        inline constexpr float kButtonHeight = 36.0f;
        inline constexpr float kNavigationHeight = 54.0f;
        inline constexpr float kListItemHeight = 48.0f;
        inline constexpr float kChipHeight = 22.0f;
        inline constexpr float kModalWidth = 420.0f;
        inline constexpr float kTableRowHeight = 44.0f;
        inline constexpr float kCardRounding = 10.0f;
        inline constexpr float kInsetRounding = 8.0f;
        inline constexpr float kFrameRounding = 8.0f;
        inline constexpr float kQrMaximumSize = 300.0f;
        inline constexpr float kQrQuietModules = 4.0f;
        inline constexpr float kMonitorTextRows = 2.0f;
        inline constexpr float kMonitorMinimumPlotHeight = 40.0f;
        inline constexpr float kMonitorMinimumBarWidth = 20.0f;
    }

    inline constexpr float kSidebarWidth = metrics::kSidebarWidth;
    inline constexpr float kCompactWidth = metrics::kCompactWidth;
    inline constexpr float kLabelWidth = metrics::kLabelWidth;

    // shadcn/ui dark neutral theme (neutral base, new-york style).
    inline constexpr ImVec4 kBackground{0.094f, 0.094f, 0.098f, 1.0f}; // #18181B zinc-900
    inline constexpr ImVec4 kSurface{0.094f, 0.094f, 0.098f, 1.0f}; // #18181B matches background
    inline constexpr ImVec4 kSurfaceRaised{0.149f, 0.149f, 0.169f, 1.0f}; // #27272A zinc-800
    inline constexpr ImVec4 kControl{1.0f, 1.0f, 1.0f, 0.15f}; // input/frame bg
    inline constexpr ImVec4 kControlHover{1.0f, 1.0f, 1.0f, 0.20f}; // input hover
    inline constexpr ImVec4 kOutline{1.0f, 1.0f, 1.0f, 0.10f}; // border
    inline constexpr ImVec4 kOutlineSoft{1.0f, 1.0f, 1.0f, 0.06f}; // subtle separator
    inline constexpr ImVec4 kText{0.980f, 0.980f, 0.976f, 1.0f}; // #FAFAFA zinc-50
    inline constexpr ImVec4 kTextMuted{0.631f, 0.631f, 0.671f, 1.0f}; // #A1A1AA zinc-400
    inline constexpr ImVec4 kPrimary{0.980f, 0.980f, 0.976f, 1.0f}; // #FAFAFA neutral primary
    inline constexpr ImVec4 kPrimaryMuted{1.0f, 1.0f, 1.0f, 0.10f}; // subtle primary hover
    inline constexpr ImVec4 kSelection{1.0f, 1.0f, 1.0f, 0.15f}; // active state
    inline constexpr ImVec4 kOnPrimary{0.094f, 0.094f, 0.098f, 1.0f}; // #18181B text on primary
    inline constexpr ImVec4 kOnSurface{0.980f, 0.980f, 0.976f, 1.0f}; // #FAFAFA text on cards
    inline constexpr ImVec4 kSuccess{0.133f, 0.773f, 0.369f, 1.0f}; // #22C55E green-500
    inline constexpr ImVec4 kWarning{0.961f, 0.620f, 0.043f, 1.0f}; // #F59E0B amber-500
    inline constexpr ImVec4 kError{0.937f, 0.267f, 0.267f, 1.0f}; // #EF4444 red-500
    inline constexpr ImVec4 kErrorMuted{0.937f, 0.267f, 0.267f, 0.14f}; // red-500 @ 14%
    inline constexpr ImVec4 kInfo{0.980f, 0.980f, 0.976f, 1.0f}; // #FAFAFA neutral info
    inline constexpr ImVec4 kAttention{0.961f, 0.620f, 0.043f, 1.0f}; // #F59E0B amber-500
    inline constexpr ImVec4 kAccent{0.631f, 0.631f, 0.671f, 1.0f}; // #A1A1AA zinc-400

    void apply_theme(float scale = 1.0f);
}

#endif //ISAACCOYOTE_THEMES_H
