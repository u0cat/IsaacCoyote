// Created by TsCat on 2026/7/25.

#include "isaac_spy/hooks/start_new_game_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Manager::StartNewGame";

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    StartNewGameHook::StartNewGameHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternManagerStartNewGame, 0},
                   &trampoline, &s_active) {}

    void StartNewGameHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<StartNewGameHook*>(hook)->handle(context);
    }

    void StartNewGameHook::handle(SafetyHookContext& context) {
        StartNewGameContext start{};
        if (!mem::safe_read_raw(&start.player_type, context.esp + kOffsetStartNewGamePlayerType, sizeof(start.player_type)) ||
            !mem::safe_read_raw(&start.challenge, context.esp + kOffsetStartNewGameChallenge, sizeof(start.challenge)) ||
            !mem::safe_read_raw(&start.difficulty, context.esp + kOffsetStartNewGameDifficulty, sizeof(start.difficulty))) {
            return;
        }

        log_().debug("[StartNewGame] player_type={} challenge={} difficulty={}",
                     start.player_type, start.challenge, start.difficulty);

        emit(start);
    }
}
