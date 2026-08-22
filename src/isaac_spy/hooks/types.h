//
// Created by TsCat on 2026/7/18.
//

#ifndef ISAACSPY_HOOK_TYPES_H
#define ISAACSPY_HOOK_TYPES_H
#include <cstdint>
#include <optional>
#include <string>

#include "isaac_spy/isaac/entity_ref.h"
#include "isaac_spy/isaac/enums.h"

namespace isaac_spy::hook
{
    enum class HookErrorCode {
        InvalidPattern,
        PatternNotFound,
        PatternNotUnique,
        DuplicateName,
        CreateFailed,
        NotFound,
    };

    struct HookError {
        HookErrorCode code;
        std::string message;
    };

    struct HurtContext {
        std::uintptr_t player = 0;
        float raw_damage = 0.0f;
        int final_damage = 0;
        isaac::enums::DamageFlag flags = isaac::enums::DamageFlag::DAMAGE_NOKILL;
        bool cancelled = false;
        std::optional<isaac::EntityRef> entity_ref;
    };

    struct StartNewGameContext {
        int player_type;
        int challenge;
        int difficulty;
    };

    struct RestartGameContext {
        bool clear_seed_effects;
        bool progress_scared_heart;
        bool from_console = false;  // restart was triggered from the in-game console
    };

    struct PlayerDeathContext {
        std::uintptr_t player = 0;
        // int extra_lives;
    };

    struct UseActiveItemContext {
        std::uintptr_t player = 0;
        int item_id = 0;
        unsigned int use_flags = 0;
        int active_slot = 0;
        int var_data = 0;
        std::uintptr_t result_flags = 0;
    };

    struct UsePillContext {
        std::uintptr_t player = 0;
        int pill_effect = 0;
        int pill_color = 0;
        int color_id = 0;
        bool golden = false;
        unsigned int use_flags = 0;
    };

    struct UseCardContext {
        std::uintptr_t player = 0;
        int card_id = 0;
        unsigned int use_flags = 0;
        std::uintptr_t card_config = 0;
        std::uintptr_t card_slot = 0;
    };

}
#endif // ISAACSPY_HOOK_TYPES_H
