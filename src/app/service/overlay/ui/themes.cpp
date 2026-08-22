//
// Created by TsCat on 2026/7/28.
//

#include "app/service/overlay/ui/themes.h"

using namespace app::overlay::ui;

void app::overlay::ui::apply_theme(const float scale) {
    metrics::current_scale() = scale;
    ImGuiStyle style{};
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(12.0f, 6.0f);
    style.ItemSpacing = ImVec2(metrics::kPageGap, metrics::kPageGap);
    style.ItemInnerSpacing = ImVec2(metrics::kSmallGap, metrics::kSmallGap);
    style.FrameRounding = metrics::kFrameRounding;
    style.GrabRounding = metrics::kFrameRounding;
    style.ChildRounding = metrics::kCardRounding;
    style.PopupRounding = metrics::kCardRounding;
    style.ScrollbarRounding = metrics::kFrameRounding;
    style.TabRounding = metrics::kFrameRounding;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    auto& colors = style.Colors;
    colors[ImGuiCol_Text] = kText;
    colors[ImGuiCol_TextDisabled] = kTextMuted;
    colors[ImGuiCol_WindowBg] = kBackground;
    colors[ImGuiCol_ChildBg] = kSurface;
    colors[ImGuiCol_PopupBg] = kSurfaceRaised;
    colors[ImGuiCol_Border] = kOutline;
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = kControl;
    colors[ImGuiCol_FrameBgHovered] = kControlHover;
    colors[ImGuiCol_FrameBgActive] = kSelection;
    colors[ImGuiCol_TitleBg] = kSurface;
    colors[ImGuiCol_TitleBgActive] = kSurfaceRaised;
    colors[ImGuiCol_TitleBgCollapsed] = kSurface;
    colors[ImGuiCol_MenuBarBg] = kSurface;
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = kOutline;
    colors[ImGuiCol_ScrollbarGrabHovered] = kOutlineSoft;
    colors[ImGuiCol_ScrollbarGrabActive] = kTextMuted;
    colors[ImGuiCol_CheckMark] = kText;
    colors[ImGuiCol_SliderGrab] = kTextMuted;
    colors[ImGuiCol_SliderGrabActive] = kText;
    colors[ImGuiCol_Button] = kControl;
    colors[ImGuiCol_ButtonHovered] = kControlHover;
    colors[ImGuiCol_ButtonActive] = kSelection;
    colors[ImGuiCol_Header] = kPrimaryMuted;
    colors[ImGuiCol_HeaderHovered] = kControlHover;
    colors[ImGuiCol_HeaderActive] = kSelection;
    colors[ImGuiCol_Separator] = kOutline;
    colors[ImGuiCol_SeparatorHovered] = kTextMuted;
    colors[ImGuiCol_SeparatorActive] = kText;
    colors[ImGuiCol_ResizeGrip] = ImVec4(kTextMuted.x, kTextMuted.y, kTextMuted.z, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(kTextMuted.x, kTextMuted.y, kTextMuted.z, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = kTextMuted;
    colors[ImGuiCol_Tab] = kSurfaceRaised;
    colors[ImGuiCol_TabHovered] = kControlHover;
    colors[ImGuiCol_TabSelected] = kSurfaceRaised;
    colors[ImGuiCol_TabSelectedOverline] = kTextMuted;
    colors[ImGuiCol_TabDimmed] = kSurface;
    colors[ImGuiCol_TabDimmedSelected] = kControlHover;
    colors[ImGuiCol_TabDimmedSelectedOverline] = kOutline;
    colors[ImGuiCol_PlotLines] = kTextMuted;
    colors[ImGuiCol_PlotLinesHovered] = kText;
    colors[ImGuiCol_PlotHistogram] = kAttention;
    colors[ImGuiCol_PlotHistogramHovered] = kWarning;
    colors[ImGuiCol_TableHeaderBg] = kSurfaceRaised;
    colors[ImGuiCol_TableBorderStrong] = kOutline;
    colors[ImGuiCol_TableBorderLight] = kOutlineSoft;
    colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
    colors[ImGuiCol_TextLink] = kInfo;
    colors[ImGuiCol_TextSelectedBg] = kSelection;
    colors[ImGuiCol_DragDropTarget] = kWarning;
    colors[ImGuiCol_NavHighlight] = kText;
    colors[ImGuiCol_NavWindowingHighlight] = kText;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(kBackground.x, kBackground.y, kBackground.z, 0.68f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(kSurface.x, kSurface.y, kSurface.z, 0.76f);

    style.ScaleAllSizes(scale);
    style.MouseCursorScale = 1.0f; // fix cursor size
    ImGui::GetStyle() = style;
}
