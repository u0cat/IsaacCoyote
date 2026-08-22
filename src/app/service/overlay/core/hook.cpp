//
// Created by TsCat on 2026/7/29.
//

#include "app/service/overlay/core/hook.h"

#include "app/service/log/log_service.h"

using namespace app::overlay;

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.overlay");
        return *logger;
    }
}

std::atomic<PlatformHook*> PlatformHook::active_hook_ = nullptr;

PlatformHook::PlatformHook(FrameCallback frame_callback, MessageCallback message_callback)
    : frame_callback_(std::move(frame_callback)), message_callback_(std::move(message_callback)),
      all_callbacks_drained_(CreateEventA(nullptr, TRUE, FALSE, nullptr)) {}

PlatformHook::~PlatformHook() {
    stopping_.store(true, std::memory_order_release);
    active_hook_.store(nullptr, std::memory_order_release);

    if (active_cb_count_.load() > 0 && all_callbacks_drained_) {
        WaitForSingleObject(all_callbacks_drained_, 1000);
    }

    if (original_wnd_proc_) {
        if (const auto set_window_long = set_window_long_ptr_fn())
            set_window_long(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_wnd_proc_));
    }
    original_wnd_proc_ = nullptr;
    window_ = nullptr;

    if (installed_) swap_buffers_hook_.reset();
    installed_ = false;

    if (all_callbacks_drained_) {
        CloseHandle(all_callbacks_drained_);
        all_callbacks_drained_ = nullptr;
    }
}

bool PlatformHook::install() {
    if (installed_) return false;

    const HMODULE gdi32 = GetModuleHandleA("gdi32.dll");
    if (!gdi32) {
        log_().error("failed to resolve gdi32.dll");
        return false;
    }

    auto swap_buffers = reinterpret_cast<void*>(GetProcAddress(gdi32, "SwapBuffers"));
    if (!swap_buffers) {
        log_().error("failed to resolve SwapBuffers");
        return false;
    }

    active_hook_.store(this, std::memory_order_release);
    swap_buffers_hook_ = safetyhook::create_inline(swap_buffers, swap_buffers_callback);
    if (!swap_buffers_hook_) {
        active_hook_.store(nullptr, std::memory_order_release);
        log_().error("failed to create SwapBuffers inline hook");
        return false;
    }

    installed_ = true;
    log_().info("SwapBuffers hook installed");
    return true;
}

bool PlatformHook::attach_window(const HWND window) {
    if (window_ == window && original_wnd_proc_) return true;
    if (window_ || !window) return false;

    const auto set_window_long = set_window_long_ptr_fn();
    if (!set_window_long) return false;

    SetLastError(0);
    original_wnd_proc_ = reinterpret_cast<WNDPROC>(
        set_window_long(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc_callback))
    );
    if (!original_wnd_proc_ && GetLastError() != 0) return false;

    window_ = window;
    return true;
}

bool PlatformHook::is_installed() const noexcept {
    return installed_;
}

BOOL PlatformHook::handle_swap_buffers(const HDC hdc) {
    frame_callback_(hdc);
    return swap_buffers_hook_.stdcall<BOOL>(hdc);
}

LRESULT PlatformHook::handle_wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam) {
    if (const auto result = message_callback_(hwnd, msg, wparam, lparam))
        return *result;
    return original_wnd_proc_
               ? CallWindowProc(original_wnd_proc_, hwnd, msg, wparam, lparam)
               : DefWindowProc(hwnd, msg, wparam, lparam);
}

BOOL WINAPI PlatformHook::swap_buffers_callback(const HDC hdc) {
    auto* hook = active_hook_.load(std::memory_order_acquire);
    if (!hook) return FALSE;

    hook->active_cb_count_.fetch_add(1, std::memory_order_acq_rel);

    if (hook->stopping_.load(std::memory_order_acquire)) {
        if (hook->active_cb_count_.fetch_sub(1, std::memory_order_acq_rel) == 1
            && hook->stopping_.load()) {
            SetEvent(hook->all_callbacks_drained_);
        }
        return FALSE;
    }

    auto result = hook->handle_swap_buffers(hdc);
    if (hook->active_cb_count_.fetch_sub(1, std::memory_order_acq_rel) == 1
        && hook->stopping_.load()) {
        SetEvent(hook->all_callbacks_drained_);
    }
    return result;
}

LRESULT CALLBACK PlatformHook::wnd_proc_callback(
    const HWND hwnd, const UINT msg, const WPARAM wparam, const LPARAM lparam) {
    auto* hook = active_hook_.load(std::memory_order_acquire);
    if (!hook) return DefWindowProc(hwnd, msg, wparam, lparam);

    hook->active_cb_count_.fetch_add(1, std::memory_order_acq_rel);

    if (hook->stopping_.load(std::memory_order_acquire)) {
        if (hook->active_cb_count_.fetch_sub(1, std::memory_order_acq_rel) == 1
            && hook->stopping_.load()) {
            SetEvent(hook->all_callbacks_drained_);
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    auto result = hook->handle_wnd_proc(hwnd, msg, wparam, lparam);
    if (hook->active_cb_count_.fetch_sub(1, std::memory_order_acq_rel) == 1
        && hook->stopping_.load()) {
        SetEvent(hook->all_callbacks_drained_);
    }
    return result;
}

PlatformHook::SetWindowLongPtrFn PlatformHook::set_window_long_ptr_fn() {
    const HMODULE user32 = GetModuleHandleA("user32.dll");
    if (!user32) return nullptr;
    return reinterpret_cast<SetWindowLongPtrFn>(GetProcAddress(user32, "SetWindowLongA"));
}
