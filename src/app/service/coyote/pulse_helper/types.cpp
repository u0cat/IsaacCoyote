//
// Created by TsCat on 2026/7/21.
//

#include "app/service/coyote/pulse_helper/types.h"

#include <format>

using namespace app::coyote::pulse;

std::string PulseFrame::to_hex() const {
    static constexpr char hex[] = "0123456789ABCDEF";

    std::string result;
    result.reserve((frequency.size() + strength.size()) * 2);

    auto append_hex = [&](auto range)
    {
        for (auto v : range) {
            result += hex[(v >> 4) & 0x0F];
            result += hex[v & 0x0F];
        }
    };

    append_hex(frequency);
    append_hex(strength);

    return result;
}

PulseFrame PulseFrame::from_hex(std::string_view hex) {
    PulseFrame frame;

    if (hex.size() < 16) return frame;
    auto char_to_nibble = [](char c) -> std::uint8_t
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 0;
    };

    for (std::size_t i = 0; i < 8; ++i) {
        const std::uint8_t byte = static_cast<std::uint8_t>(
            (char_to_nibble(hex[i * 2]) << 4) | char_to_nibble(hex[i * 2 + 1])
        );
        if (i < 4)
            frame.frequency[i] = byte;
        else
            frame.strength[i - 4] = byte;
    }
    return frame;
}
