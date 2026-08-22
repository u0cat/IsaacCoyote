// Created by TsCat on 2026/7/26.

#include "isaac_spy/hooks/player_death_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Entity_Player::TriggerDeath";

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    PlayerDeathHook::PlayerDeathHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternEntityPlayerTriggerDeath, 0},
                   &trampoline, &s_active) {}

    void PlayerDeathHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<PlayerDeathHook*>(hook)->handle(context);
    }

    void PlayerDeathHook::handle(SafetyHookContext& context) {
        PlayerDeathContext ctx{};
        ctx.player = context.edi;

        log_().debug("[PlayerDeath] player={:#x}", ctx.player);

        emit(ctx);
    }
}
