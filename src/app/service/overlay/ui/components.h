//
// Created by TsCat on 2026/7/28.
//

#ifndef ISAACCOYOTE_UI_COMPONENTS_H
#define ISAACCOYOTE_UI_COMPONENTS_H

#include <string>

#include <imgui.h>

#include "app/service/overlay/ui/themes.h"

namespace app::overlay::ui
{
    inline constexpr float kPageGap = metrics::kPageGap;

    enum class ToastType {
        Success,
        Error
    };

    enum class ConfirmResult {
        None,
        Cancelled,
        Confirmed
    };

    class Toast {
    public:
        void notify(std::string message, ToastType type = ToastType::Success);
        void draw();

    private:
        std::string text_;
        double started_at_ = -1.0;
        ToastType type_ = ToastType::Success;
    };

    // Typography
    void muted_text(const char* text);
    void title_text(const char* text);
    void section_header(const char* title, const char* description = nullptr);

    // Actions
    bool primary_button(const char* label, ImVec2 size = ImVec2(0.0f, 0.0f));
    bool danger_button(const char* label, ImVec2 size = ImVec2(0.0f, 0.0f));
    bool tonal_button(const char* label, ImVec2 size = ImVec2(0.0f, 0.0f));

    // Layout
    void gap(float height = metrics::kPageGap);
    void spring(float reserved_height = 0.0f);
    float fill_height(float reserved_height = 0.0f, float minimum_height = 0.0f);
    float table_height(int visible_rows, bool include_header = true);
    bool is_compact(float breakpoint = metrics::kCompactWidth);
    void center_next_item(float item_width);
    int responsive_columns(float minimum_column_width, int preferred_columns = 2);
    bool begin_columns(const char* id, float minimum_column_width, int preferred_columns = 2,
                       ImGuiTableFlags extra_flags = ImGuiTableFlags_None);
    void next_column();
    void end_columns();
    bool begin_surface_card(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f), bool auto_resize_y = true,
                            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None);
    void end_surface_card();
    bool begin_inset_panel(const char* id, ImVec2 size = ImVec2(0.0f, 0.0f), bool auto_resize_y = true,
                           ImGuiWindowFlags window_flags = ImGuiWindowFlags_None);
    void end_inset_panel();
    bool page_header(const char* title, const char* description, const char* status, const ImVec4& status_color,
                     const char* action_label, bool action_enabled = true);

    // Navigation and display
    bool nav_item(const char* label, bool selected);
    bool navigation_item(const char* id, const char* label, const char* hint, bool selected, int badge = -1);
    bool list_navigation_item(const char* id, const char* label, const char* hint, bool selected,
                              const char* trailing = nullptr);
    void status_chip(const char* id, const char* text, const ImVec4& color);
    void metric_card(const char* id, const char* label, const std::string& value, const char* hint);
    void empty_state(const char* title, const char* description);

    // Forms
    bool begin_property_table(const char* id, float label_width = kLabelWidth);
    void end_property_table();
    void property_row(const char* label);
    std::string hidden_id(const char* label);
    bool input_text(const char* label, const char* hint, std::string* value);
    bool input_multiline(const char* label, std::string* value, ImVec2 size);
    bool combo(const char* label, int* value, const char* const items[], int count);
    bool combo(const char* label, int* value, const char* items_zero_separated);

    // Dialogs and data tables
    ConfirmResult confirm_modal(const char* id, bool open, const char* title, const char* description,
                                const char* confirm_label = "确认");
    bool begin_data_table(const char* id, int columns, ImVec2 size = ImVec2(0.0f, 0.0f),
                          ImGuiTableFlags extra_flags = ImGuiTableFlags_None);
    void end_data_table();
    void table_empty_row(int columns, const char* message);
}

#endif // ISAACCOYOTE_UI_COMPONENTS_H
