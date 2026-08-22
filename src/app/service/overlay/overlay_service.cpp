//
// Created by TsCat on 2026/7/7.
//

#include "app/service/overlay/overlay_service.h"

#include "app/isaac_coyote.h"
#include "app/service/log/log_service.h"
#include "app/service/overlay/core/hook.h"
#include "app/service/overlay/core/imgui_renderer.h"
#include "app/service/overlay/overlay_ui.h"

extern void console_toggle();

using namespace app::overlay;

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.overlay");
        return *logger;
    }
}

OverlayService::OverlayService(game::Game& game, config::ConfigService& config) {
    shutdown_completed_ = CreateEventA(nullptr, TRUE, FALSE, nullptr);

    ui_ = std::make_unique<OverlayUi>(game, config);
    platform_ = std::make_unique<PlatformHook>(
        [this](const HDC hdc) { render_frame(hdc); },
        [this](const HWND window, const UINT msg, const WPARAM wparam, const LPARAM lparam)
            -> std::optional<LRESULT> {
            return handle_message(window, msg, wparam, lparam);
        }
    );
    renderer_ = std::make_unique<ImGuiRenderer>(config, *platform_);
    platform_->install();
}

OverlayService::~OverlayService() {
    shutdown_requested_.store(true, std::memory_order_release);

    if (shutdown_completed_) {
        WaitForSingleObject(shutdown_completed_, 2000);
    }

    platform_.reset();
    renderer_.reset();
    ui_.reset();

    if (shutdown_completed_) {
        CloseHandle(shutdown_completed_);
        shutdown_completed_ = nullptr;
    }
}

bool OverlayService::is_hook_installed() const {
    return platform_ && platform_->is_installed();
}

bool OverlayService::is_initialized() const {
    return is_hook_installed() && renderer_ && renderer_->is_initialized();
}

void OverlayService::render_frame(const HDC hdc) {
    if (shutdown_requested_.load(std::memory_order_acquire)) {
        renderer_->shutdown();
        if (shutdown_completed_) SetEvent(shutdown_completed_);
        return;
    }
    if (renderer_->begin_frame(hdc)) {
        static bool initialized_logged = false;
        if (!initialized_logged) {
            initialized_logged = true;
            log_().info("overlay initialized");
        }
        ui_->render();
        renderer_->end_frame();
    }
    app::IsaacCoyote::get_instance().tick();
}

std::optional<LRESULT> OverlayService::handle_message(
    const HWND window, const UINT msg, const WPARAM wparam, const LPARAM lparam) {
    if (renderer_->is_initialized()) {
        if (const auto result = renderer_->handle_message(window, msg, wparam, lparam))
            return result;
    }

    if (msg == WM_KEYUP) {
        if (wparam == VK_F2) console_toggle();
        else ui_->handle_key_up(static_cast<int>(wparam));
    }
    return std::nullopt;
}
