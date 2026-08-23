//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_CONTEXT_H
#define ISAACCOYOTE_CONTEXT_H

#include <cstdint>
#include <format>
#include <string>

namespace app::event
{
    using PlayerId = std::string;

    inline PlayerId ptr_player_id(const std::uintptr_t address) {
        return PlayerId{std::format("{:x}", address)};
    }

    enum class PlayerRelation {
        Self,
        Other,
    };

    struct EventContext {
        PlayerId player_id;
        PlayerRelation relation = PlayerRelation::Self;
        std::string player_name;
    };
}

#endif //ISAACCOYOTE_CONTEXT_H
