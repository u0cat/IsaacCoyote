//
// Created by TsCat on 2026/7/29.
//

#ifndef ISAACCOYOTE_IMGUI_RENDERER_H
#define ISAACCOYOTE_IMGUI_RENDERER_H

#include <windows.h>
#include <optional>
#include <string>

#include "app/service/config/config_service.h"

namespace app::overlay
{
    class PlatformHook;

    class ImGuiRenderer {
    public:
        ImGuiRenderer(config::ConfigService& config, PlatformHook& platform);
        ~ImGuiRenderer();

        bool begin_frame(HDC hdc);
        void end_frame();
        std::optional<LRESULT> handle_message(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) const;
        [[nodiscard]] bool is_initialized() const noexcept;
        void shutdown();

    private:
        bool initialize(HDC hdc);
        void update_scale();

        config::ConfigService& config_;
        PlatformHook& platform_;
        std::string ini_path_ = "isaac-coyote-imgui.ini";
        HWND window_ = nullptr;
        float applied_scale_ = 0.0f;
        bool initialized_ = false;
        bool initialization_failed_ = false;
    };
}

#endif // ISAACCOYOTE_IMGUI_RENDERER_H
