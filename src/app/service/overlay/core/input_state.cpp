//
// Created by TsCat on 2026/7/29.
//

#include "app/service/overlay/core/input_state.h"

using namespace app::overlay;

bool InputState::handle_key_up(const int key, const int menu_key) noexcept {
    if (capturing_shortcut_) {
        if (!is_bindable_key(key)) return false;
        captured_shortcut_ = key;
        capturing_shortcut_ = false;
        return true;
    }

    if (key != menu_key)
        return false;

    menu_open_ = !menu_open_;
    return true;
}

void InputState::begin_shortcut_capture() noexcept {
    captured_shortcut_.reset();
    capturing_shortcut_ = true;
}

void InputState::cancel_shortcut_capture() noexcept {
    capturing_shortcut_ = false;
    captured_shortcut_.reset();
}

bool InputState::is_capturing_shortcut() const noexcept {
    return capturing_shortcut_;
}

std::optional<int> InputState::take_captured_shortcut() noexcept {
    auto captured = captured_shortcut_;
    captured_shortcut_.reset();
    return captured;
}

bool InputState::is_menu_open() const noexcept {
    return menu_open_;
}

bool InputState::is_bindable_key(const int key) noexcept {
    return key > 0 && key <= 0xFF && key != kConsoleKey && key != kUnloadKey;
}
