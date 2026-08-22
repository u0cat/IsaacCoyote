//
// Created by TsCat on 2026/7/29.
//

#ifndef ISAACCOYOTE_OVERLAY_PLATFORM_HOOK_H
#define ISAACCOYOTE_OVERLAY_PLATFORM_HOOK_H

#include <windows.h>
#include <atomic>
#include <functional>
#include <optional>

#include <safetyhook.hpp>

namespace app::overlay
{
    class PlatformHook {
    public:
        using FrameCallback = std::function<void(HDC)>;
        using MessageCallback = std::function<std::optional<LRESULT>(HWND, UINT, WPARAM, LPARAM)>;
        using SetWindowLongPtrFn = LONG_PTR(WINAPI*)(HWND, int, LONG_PTR);

        PlatformHook(FrameCallback frame_callback, MessageCallback message_callback);
        ~PlatformHook();

        bool install();
        bool attach_window(HWND window);
        [[nodiscard]] bool is_installed() const noexcept;

    private:
        BOOL handle_swap_buffers(HDC hdc);
        LRESULT handle_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        static BOOL WINAPI swap_buffers_callback(HDC hdc);
        static LRESULT CALLBACK wnd_proc_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        static SetWindowLongPtrFn set_window_long_ptr_fn();

        static std::atomic<PlatformHook*> active_hook_;

        FrameCallback frame_callback_;
        MessageCallback message_callback_;
        safetyhook::InlineHook swap_buffers_hook_{};
        HWND window_ = nullptr;
        WNDPROC original_wnd_proc_ = nullptr;
        bool installed_ = false;

        std::atomic<int> active_cb_count_{0};
        std::atomic<bool> stopping_{false};
        HANDLE all_callbacks_drained_ = nullptr;
    };
}

#endif // ISAACCOYOTE_OVERLAY_PLATFORM_HOOK_H
