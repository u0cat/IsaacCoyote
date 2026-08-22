//
// Created by TsCat on 2026/7/7.
//

#ifndef ISAACCOYOTE_OVERLAY_H
#define ISAACCOYOTE_OVERLAY_H

#include <windows.h>
#include <atomic>
#include <memory>
#include <optional>

#include "app/game.h"

namespace app::config
{
    class ConfigService;
}

namespace app::overlay
{
    class ImGuiRenderer;
    class OverlayUi;
    class PlatformHook;

    class OverlayService {
    public:
        OverlayService(game::Game& game, config::ConfigService& config);
        ~OverlayService();

        bool is_hook_installed() const;
        bool is_initialized() const;

    private:
        void render_frame(HDC hdc);
        std::optional<LRESULT> handle_message(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

        std::unique_ptr<PlatformHook> platform_;
        std::unique_ptr<ImGuiRenderer> renderer_;
        std::unique_ptr<OverlayUi> ui_;

        std::atomic<bool> shutdown_requested_{false};
        HANDLE shutdown_completed_ = nullptr;
    };
}

#endif // ISAACCOYOTE_OVERLAY_H
