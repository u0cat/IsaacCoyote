//
// Created by TsCat on 2026/7/28.
//

#include "app/service/overlay/ui/components.h"

#include <algorithm>
#include <imgui_internal.h>

#include <imgui_stdlib.h>

#include "app/service/overlay/ui/themes.h"

using namespace app::overlay::ui;

namespace
{
    ImVec4 with_alpha(ImVec4 color, const float alpha) {
        color.w = alpha;
        return color;
    }

    ImVec4 blend(const ImVec4& a, const ImVec4& b, const float amount) {
        return ImVec4(
            a.x + (b.x - a.x) * amount,
            a.y + (b.y - a.y) * amount,
            a.z + (b.z - a.z) * amount,
            a.w + (b.w - a.w) * amount
        );
    }
}

void Toast::notify(std::string message, const ToastType type) {
    text_ = std::move(message);
    started_at_ = ImGui::GetTime();
    type_ = type;
}

namespace
{
    void draw_toast_icon(ImDrawList* dl, ImVec2 center, ImU32 color, const bool error) {
        const float r = metrics::scale(10.0f);
        dl->AddCircleFilled(center, r, ImGui::GetColorU32(with_alpha(kSurfaceRaised, 0.15f)));

        if (error) {
            dl->AddLine({center.x, center.y - metrics::scale(4.0f)}, {center.x, center.y + metrics::scale(1.5f)}, color, metrics::scale(1.8f));
            dl->AddCircleFilled({center.x, center.y + metrics::scale(4.5f)}, metrics::scale(1.1f), color);
        } else {
            dl->AddLine({center.x - metrics::scale(4.0f), center.y}, {center.x - metrics::scale(1.0f), center.y + metrics::scale(3.0f)}, color, metrics::scale(1.8f));
            dl->AddLine({center.x - metrics::scale(1.0f), center.y + metrics::scale(3.0f)}, {center.x + metrics::scale(4.5f), center.y - metrics::scale(3.5f)}, color, metrics::scale(1.8f));
        }
    }
}

void Toast::draw() {
    if (text_.empty() || started_at_ < 0.0) return;

    constexpr double dur = 4.0, fadeIn = 0.18, fadeOut = 0.30;
    const double elapsed = ImGui::GetTime() - started_at_;
    if (elapsed >= dur) {
        text_.clear();
        started_at_ = -1.0;
        return;
    }

    auto sat = [](double v) { return (float)std::clamp(v, 0.0, 1.0); };
    auto ss  = [](float t) { return t * t * (3.0f - 2.0f * t); };

    const float enter = ss(sat(elapsed / fadeIn));
    const float exit  = sat((dur - elapsed) / fadeOut);
    const float alpha = std::min(enter, exit);

    const bool error = type_ == ToastType::Error;
    const ImVec4 accent = error ? kError : kSuccess;
    auto* vp = ImGui::GetMainViewport();

    const float w = std::min(metrics::scale(360.0f), std::max(1.0f, vp->WorkSize.x - metrics::scale(32.0f)));
    ImVec2 pos(vp->WorkPos.x + metrics::scale(16.0f) - (1.0f - enter) * metrics::scale(8.0f),
               vp->WorkPos.y + vp->WorkSize.y - metrics::scale(16.0f));

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, {0.0f, 1.0f});
    ImGui::SetNextWindowSize({w, 0.0f}, ImGuiCond_Always);
#ifdef IMGUI_HAS_VIEWPORT
    ImGui::SetNextWindowViewport(vp->ID);
#endif

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, metrics::scale(ImVec2(16.0f, 14.0f)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, metrics::kCardRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, metrics::scale(1.0f));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, with_alpha(kSurfaceRaised, 0.95f * alpha));
    ImGui::PushStyleColor(ImGuiCol_Border,    with_alpha(kOutline,       alpha));
    ImGui::PushStyleColor(ImGuiCol_Text,      with_alpha(kText,          alpha));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoInputs;
#ifdef IMGUI_HAS_DOCK
    flags |= ImGuiWindowFlags_NoDocking;
#endif

    if (ImGui::Begin("##toast", nullptr, flags)) {
        const float bsz = metrics::scale(20.0f), sp = metrics::scale(12.0f);
        ImVec2 cur = ImGui::GetCursorScreenPos();
        ImVec2 center(cur.x + bsz * 0.5f, cur.y + bsz * 0.5f);

        draw_toast_icon(ImGui::GetWindowDrawList(), center,
                        ImGui::GetColorU32(with_alpha(accent, alpha)), error);

        ImGui::Dummy({bsz, bsz});
        ImGui::SameLine(0.0f, sp);
        ImGui::BeginGroup();

        ImGui::PushStyleColor(ImGuiCol_Text, with_alpha(kText, alpha));
        ImGui::TextUnformatted(error ? "Error" : "Success");
        ImGui::PopStyleColor();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + metrics::scale(2.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, with_alpha(kTextMuted, alpha));
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(text_.c_str());
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();

        ImGui::EndGroup();
    }
    ImGui::End();

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);
}

void app::overlay::ui::muted_text(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, kTextMuted);
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
}

void app::overlay::ui::title_text(const char* text) {
    ImGui::SetWindowFontScale(1.12f);
    ImGui::TextUnformatted(text);
    ImGui::SetWindowFontScale(1.0f);
}

void app::overlay::ui::gap(const float height) {
    ImGui::Dummy(ImVec2(0.0f, height));
}

void app::overlay::ui::spring(const float reserved_height) {
    const float height = ImGui::GetContentRegionAvail().y - reserved_height;
    if (height > 0.0f) gap(height);
}

float app::overlay::ui::fill_height(const float reserved_height, const float minimum_height) {
    return std::max(minimum_height, ImGui::GetContentRegionAvail().y - reserved_height);
}

float app::overlay::ui::table_height(const int visible_rows, const bool include_header) {
    const float row_height = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().CellPadding.y * 2.0f + 8.0f;
    return row_height * static_cast<float>(visible_rows + (include_header ? 1 : 0));
}

bool app::overlay::ui::is_compact(const float breakpoint) {
    return ImGui::GetContentRegionAvail().x < breakpoint;
}

void app::overlay::ui::center_next_item(const float item_width) {
    const float offset = std::max(0.0f, (ImGui::GetContentRegionAvail().x - item_width) * 0.5f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
}

int app::overlay::ui::responsive_columns(const float minimum_column_width, const int preferred_columns) {
    const float spacing = ImGui::GetStyle().ItemSpacing.x;
    const int available_columns = static_cast<int>((ImGui::GetContentRegionAvail().x + spacing) /
                                                   (minimum_column_width + spacing));
    return std::clamp(available_columns, 1, std::max(preferred_columns, 1));
}

bool app::overlay::ui::begin_columns(const char* id, const float minimum_column_width, const int preferred_columns,
                                     const ImGuiTableFlags extra_flags) {
    const int columns = responsive_columns(minimum_column_width, preferred_columns);
    if (!ImGui::BeginTable(id, columns, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoSavedSettings |
                                       extra_flags)) return false;
    for (int column = 0; column < columns; ++column)
        ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
    return true;
}

void app::overlay::ui::next_column() {
    ImGui::TableNextColumn();
}

void app::overlay::ui::end_columns() {
    ImGui::EndTable();
}

bool app::overlay::ui::primary_button(const char* label, const ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_Button, kPrimary);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blend(kPrimary, ImVec4(1,1,1,1), 0.10f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, blend(kPrimary, ImVec4(0,0,0,1), 0.10f));
    ImGui::PushStyleColor(ImGuiCol_Text, kOnPrimary);
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return pressed;
}

bool app::overlay::ui::tonal_button(const char* label, const ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_Button, kControl);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kControlHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kSelection);
    ImGui::PushStyleColor(ImGuiCol_Text, kText);
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return pressed;
}

bool app::overlay::ui::danger_button(const char* label, const ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_Button, kError);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, blend(kError, ImVec4(1,1,1,1), 0.10f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, blend(kError, ImVec4(0,0,0,1), 0.10f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    const bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return pressed;
}

bool app::overlay::ui::nav_item(const char* label, const bool selected) {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, metrics::scale(ImVec2(10.0f, 8.0f)));
    if (selected) ImGui::PushStyleColor(ImGuiCol_Text, kPrimary);
    const bool pressed = ImGui::Selectable(label, selected, ImGuiSelectableFlags_SpanAvailWidth,
                                           ImVec2(0.0f, metrics::kButtonHeight));
    if (selected) ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    return pressed;
}

bool app::overlay::ui::begin_surface_card(const char* id, const ImVec2 size, const bool auto_resize_y,
                                          const ImGuiWindowFlags window_flags) {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics::kCardRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, metrics::scale(1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(metrics::kSectionGap, metrics::kSectionGap));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSurface);
    ImGui::PushStyleColor(ImGuiCol_Border, kOutline);

    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (auto_resize_y) flags |= ImGuiChildFlags_AutoResizeY;
    return ImGui::BeginChild(id, size, flags, window_flags);
}

void app::overlay::ui::end_surface_card() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

bool app::overlay::ui::begin_inset_panel(const char* id, const ImVec2 size, const bool auto_resize_y,
                                         const ImGuiWindowFlags window_flags) {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, metrics::kInsetRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, metrics::scale(1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(metrics::kSectionGap, metrics::kSectionGap));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kSurfaceRaised);
    ImGui::PushStyleColor(ImGuiCol_Border, kOutlineSoft);

    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (auto_resize_y) flags |= ImGuiChildFlags_AutoResizeY;
    return ImGui::BeginChild(id, size, flags, window_flags);
}

void app::overlay::ui::end_inset_panel() {
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

void app::overlay::ui::status_chip(const char* id, const char* text, const ImVec4& color) {
    ImGui::PushID(id);
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const ImVec2 size(text_size.x + metrics::scale(34.0f), metrics::kChipHeight);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max(min.x + size.x, min.y + size.y);

    ImGui::InvisibleButton("##chip", size);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(min, max, ImGui::GetColorU32(with_alpha(color, 0.14f)), size.y * 0.5f);
    draw_list->AddCircleFilled(ImVec2(min.x + metrics::scale(13.0f), min.y + size.y * 0.5f), metrics::scale(3.0f), ImGui::GetColorU32(color));
    draw_list->AddText(ImVec2(min.x + metrics::scale(22.0f), min.y + (size.y - text_size.y) * 0.5f),
                       ImGui::GetColorU32(color), text);
    ImGui::PopID();
}

bool app::overlay::ui::navigation_item(const char* id, const char* label, const char* hint, const bool selected,
                                       const int badge) {
    ImGui::PushID(id);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, metrics::kNavigationHeight);
    ImGui::InvisibleButton("##navigation_item", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();

    const ImVec4 accent = ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
    auto* draw_list = ImGui::GetWindowDrawList();
    if (selected || hovered) {
        draw_list->AddRectFilled(min, ImVec2(min.x + size.x, min.y + size.y),
                                 ImGui::GetColorU32(with_alpha(accent, selected ? 0.16f : 0.07f)),
                                 metrics::kInsetRounding);
    }
    if (selected) {
        draw_list->AddRectFilled(ImVec2(min.x, min.y + metrics::scale(10.0f)), ImVec2(min.x + metrics::scale(3.0f), min.y + size.y - metrics::scale(10.0f)),
                                 ImGui::GetColorU32(accent), metrics::scale(2.0f));
    }

    const ImU32 title_color = ImGui::GetColorU32(selected ? accent : ImGui::GetStyleColorVec4(ImGuiCol_Text));
    draw_list->AddText(ImVec2(min.x + metrics::scale(14.0f), min.y + metrics::scale(9.0f)), title_color, label);
    draw_list->AddText(ImVec2(min.x + metrics::scale(14.0f), min.y + metrics::scale(30.0f)), ImGui::GetColorU32(kTextMuted), hint);

    if (badge >= 0) {
        const std::string badge_text = std::to_string(badge);
        const ImVec2 badge_text_size = ImGui::CalcTextSize(badge_text.c_str());
        const float badge_width = std::max(metrics::scale(24.0f), badge_text_size.x + metrics::scale(12.0f));
        const ImVec2 badge_min(min.x + size.x - badge_width - metrics::scale(10.0f), min.y + metrics::scale(15.0f));
        const ImVec2 badge_max(badge_min.x + badge_width, badge_min.y + metrics::scale(24.0f));
        draw_list->AddRectFilled(badge_min, badge_max,
                                 ImGui::GetColorU32(with_alpha(accent, selected ? 0.22f : 0.10f)), metrics::scale(12.0f));
        draw_list->AddText(ImVec2(badge_min.x + (badge_width - badge_text_size.x) * 0.5f,
                                  badge_min.y + (metrics::scale(24.0f) - badge_text_size.y) * 0.5f),
                           title_color, badge_text.c_str());
    }

    ImGui::PopID();
    return clicked;
}

bool app::overlay::ui::list_navigation_item(const char* id, const char* label, const char* hint,
                                             const bool selected, const char* trailing) {
    ImGui::PushID(id);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 size(ImGui::GetContentRegionAvail().x, metrics::kListItemHeight);
    ImGui::InvisibleButton("##list_navigation_item", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool clicked = ImGui::IsItemClicked();
    auto* draw_list = ImGui::GetWindowDrawList();
    if (selected || hovered) {
        draw_list->AddRectFilled(min, ImVec2(min.x + size.x, min.y + size.y),
                                 ImGui::GetColorU32(with_alpha(kPrimary, selected ? 0.16f : 0.07f)),
                                 metrics::kInsetRounding);
    }
    if (selected) {
        draw_list->AddRectFilled(ImVec2(min.x, min.y + metrics::scale(9.0f)), ImVec2(min.x + metrics::scale(3.0f), min.y + size.y - metrics::scale(9.0f)),
                                 ImGui::GetColorU32(kPrimary), metrics::scale(2.0f));
    }
    draw_list->AddText(ImVec2(min.x + metrics::scale(13.0f), min.y + metrics::scale(7.0f)),
                       ImGui::GetColorU32(selected ? kPrimary : kText), label);
    draw_list->AddText(ImVec2(min.x + metrics::scale(13.0f), min.y + metrics::scale(27.0f)), ImGui::GetColorU32(kTextMuted), hint);
    if (trailing && trailing[0] != '\0') {
        const ImVec2 text_size = ImGui::CalcTextSize(trailing);
        draw_list->AddText(ImVec2(min.x + size.x - text_size.x - metrics::scale(12.0f), min.y + metrics::scale(15.0f)),
                           ImGui::GetColorU32(kTextMuted), trailing);
    }
    ImGui::PopID();
    return clicked;
}

bool app::overlay::ui::page_header(const char* title, const char* description, const char* status,
                                   const ImVec4& status_color, const char* action_label,
                                   const bool action_enabled) {
    bool save = false;
    const float width = ImGui::GetContentRegionAvail().x;

    if (begin_surface_card("##page_header")) {
        if (width >= metrics::kTwoColumnWidth &&
            ImGui::BeginTable("##page_header_layout", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("title", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed, metrics::scale(250.0f));
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::SetWindowFontScale(1.32f);
            ImGui::TextUnformatted(title);
            ImGui::SetWindowFontScale(1.0f);
            muted_text(description);

            ImGui::TableSetColumnIndex(1);
            ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), ImGui::GetWindowContentRegionMax().x - metrics::scale(246.0f)));
            status_chip("status", status, status_color);
            ImGui::SameLine(0.0f, metrics::scale(8.0f));
            ImGui::BeginDisabled(!action_enabled);
            save = primary_button(action_label, ImVec2(metrics::scale(112.0f), metrics::kButtonHeight));
            ImGui::EndDisabled();
            ImGui::EndTable();
        }
        else {
            ImGui::SetWindowFontScale(1.26f);
            ImGui::TextUnformatted(title);
            ImGui::SetWindowFontScale(1.0f);
            muted_text(description);
            ImGui::Dummy(ImVec2(0.0f, metrics::scale(7.0f)));
            status_chip("status", status, status_color);
            ImGui::SameLine(0.0f, metrics::scale(8.0f));
            ImGui::BeginDisabled(!action_enabled);
            save = primary_button(action_label, ImVec2(metrics::scale(112.0f), metrics::kButtonHeight));
            ImGui::EndDisabled();
        }
    }
    end_surface_card();
    ImGui::Dummy(ImVec2(0.0f, kPageGap));
    return save;
}

void app::overlay::ui::metric_card(const char* id, const char* label, const std::string& value, const char* hint) {
    if (begin_surface_card(id)) {
        ImGui::PushStyleColor(ImGuiCol_Text, kTextMuted);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0.0f, metrics::scale(4.0f)));
        ImGui::SetWindowFontScale(1.42f);
        ImGui::TextUnformatted(value.c_str());
        ImGui::SetWindowFontScale(1.0f);
        muted_text(hint);
    }
    end_surface_card();
}

void app::overlay::ui::section_header(const char* title, const char* description) {
    ImGui::Dummy(ImVec2(0.0f, metrics::scale(4.0f)));
    title_text(title);
    if (description != nullptr && description[0] != '\0') muted_text(description);
    ImGui::Dummy(ImVec2(0.0f, metrics::scale(3.0f)));
}

bool app::overlay::ui::begin_property_table(const char* id, const float label_width) {
    if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg)) return false;
    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, label_width);
    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    return true;
}

void app::overlay::ui::end_property_table() {
    ImGui::EndTable();
}

void app::overlay::ui::property_row(const char* label) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, kTextMuted);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1.0f);
}

std::string app::overlay::ui::hidden_id(const char* label) {
    return std::string("##") + label;
}

bool app::overlay::ui::input_text(const char* label, const char* hint, std::string* value) {
    const std::string id = hidden_id(label);
    ImGui::SetNextItemWidth(-1.0f);
    return ImGui::InputTextWithHint(id.c_str(), hint, value);
}

bool app::overlay::ui::input_multiline(const char* label, std::string* value, const ImVec2 size) {
    const std::string id = hidden_id(label);
    return ImGui::InputTextMultiline(id.c_str(), value, size);
}

bool app::overlay::ui::combo(const char* label, int* value, const char* const items[], const int count) {
    const std::string id = hidden_id(label);
    ImGui::SetNextItemWidth(-1.0f);
    return ImGui::Combo(id.c_str(), value, items, count);
}

bool app::overlay::ui::combo(const char* label, int* value, const char* items_zero_separated) {
    const std::string id = hidden_id(label);
    ImGui::SetNextItemWidth(-1.0f);
    return ImGui::Combo(id.c_str(), value, items_zero_separated);
}

ConfirmResult app::overlay::ui::confirm_modal(const char* id, const bool open,
                                              const char* title, const char* description,
                                              const char* confirm_label
                                              ) {
    if (open) ImGui::OpenPopup(id);
    const float padding = metrics::scale(24.0f);
    const float cancel_width = metrics::scale(92.0f);
    const float submit_width = metrics::scale(132.0f);
    const float button_height = metrics::scale(36.0f);
    const float button_spacing = ImGui::GetStyle().ItemSpacing.x;

    const float buttons_width =
        cancel_width +
        button_spacing +
        submit_width;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(metrics::kModalWidth, 0.0f), ImGuiCond_Appearing);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, metrics::kCardRounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, metrics::scale(1.0f));
    ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(kSurface.x, kSurface.y, kSurface.z, 0.76f));

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoTitleBar;

    ConfirmResult result = ConfirmResult::None;

    if (ImGui::BeginPopupModal(id, nullptr, flags)) {
        title_text(title);

        ImGui::PushStyleColor(ImGuiCol_Text, kTextMuted);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(description);
        ImGui::PopTextWrapPos();
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f, metrics::scale(8.0f)));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, metrics::scale(8.0f)));

        const float button_group_x =ImGui::GetCursorPosX() +
            std::max(0.0f,ImGui::GetContentRegionAvail().x - buttons_width);

        ImGui::SetCursorPosX(button_group_x);
        if (tonal_button("取消",ImVec2(cancel_width, button_height))) result = ConfirmResult::Cancelled;
        ImGui::SameLine(0.0f, button_spacing);
        if (danger_button(confirm_label,ImVec2(submit_width, button_height))) result = ConfirmResult::Confirmed;
        if (result != ConfirmResult::None) ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);

    return result;
}

bool app::overlay::ui::begin_data_table(const char* id, const int columns, const ImVec2 size,
                                         const ImGuiTableFlags extra_flags) {
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, metrics::scale(ImVec2(12.0f, 9.0f)));
    const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH |
                                  ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
                                  ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp | extra_flags;
    const bool opened = ImGui::BeginTable(id, columns, flags, size);
    if (!opened) ImGui::PopStyleVar();
    return opened;
}

void app::overlay::ui::end_data_table() {
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

void app::overlay::ui::table_empty_row(const int columns, const char* message) {
    ImGui::TableNextRow(ImGuiTableRowFlags_None, metrics::kTableRowHeight);
    ImGui::TableSetColumnIndex(0);
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(kSurfaceRaised));
    ImGui::TextDisabled("%s", message);
    for (int column = 1; column < columns; ++column) ImGui::TableNextColumn();
}

void app::overlay::ui::empty_state(const char* title, const char* description) {
    ImGui::Dummy(ImVec2(0.0f, metrics::scale(30.0f)));
    title_text(title);
    muted_text(description);
}
