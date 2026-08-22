//
// Created by TsCat on 2026/7/21.
//

#ifndef ISAACCOYOTE_APP_COYOTE_PULSE_HELPER_TYPES_H
#define ISAACCOYOTE_APP_COYOTE_PULSE_HELPER_TYPES_H
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace app::coyote::pulse
{
    enum class Channel {
        A,
        B,
    };

    struct PulseFrame {
        std::array<std::uint8_t, 4> frequency{10, 10, 10, 10};
        std::array<std::uint8_t, 4> strength{0, 0, 0, 0};

        std::string to_hex() const;
        static PulseFrame from_hex(std::string_view hex);
    };

    using PulseTrack = std::vector<PulseFrame>;
}
#endif // ISAACCOYOTE_APP_COYOTE_PULSE_HELPER_TYPES_H
