//
// Created by TsCat on 2026/7/29.
//

#ifndef ISAACCOYOTE_OVERLAY_INPUT_STATE_H
#define ISAACCOYOTE_OVERLAY_INPUT_STATE_H

#include <optional>

namespace app::overlay
{
    inline constexpr int kDefaultMenuKey = 0x2D; // VK_INSERT
    inline constexpr int kConsoleKey = 0x71; // VK_F2
    inline constexpr int kUnloadKey = 0x23; // VK_END

    class InputState {
    public:
        bool handle_key_up(int key, int menu_key) noexcept;

        void begin_shortcut_capture() noexcept;
        void cancel_shortcut_capture() noexcept;
        [[nodiscard]] bool is_capturing_shortcut() const noexcept;
        std::optional<int> take_captured_shortcut() noexcept;

        [[nodiscard]] bool is_menu_open() const noexcept;

    private:
        static bool is_bindable_key(int key) noexcept;

        bool menu_open_ = false;
        bool capturing_shortcut_ = false;
        std::optional<int> captured_shortcut_;
    };
}

#endif // ISAACCOYOTE_OVERLAY_INPUT_STATE_H
