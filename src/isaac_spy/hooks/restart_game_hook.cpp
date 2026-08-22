// Created by TsCat on 2026/7/25.

#include "isaac_spy/hooks/restart_game_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Manager::RestartGame";

        uintptr_t kConsoleRetStart = 0x0;
        uintptr_t kConsoleRetEnd = 0x0;

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    RestartGameHook::RestartGameHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternManagerRestartGame, 0},
                   &trampoline, &s_active) {}

    void RestartGameHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<RestartGameHook*>(hook)->handle(context);
    }

    void RestartGameHook::on_first_subscribe() {
        if (auto scan = mem::Scanner(kPatternManagerRestartGameConsoleStart).scan(); !scan.found) {
            log_().error("failed to find console start ptr");
        }
        else {
            kConsoleRetStart = reinterpret_cast<uintptr_t>(scan.address);
        }

        if (auto scan = mem::Scanner(kPatternManagerRestartGameConsoleEnd).scan(false, true); !scan.found) {
            log_().error("failed to find console end ptr");
        }
        else {
            kConsoleRetEnd = reinterpret_cast<uintptr_t>(scan.address);
        }
    }

    void RestartGameHook::handle(SafetyHookContext& context) {
        const bool from_console = *(uintptr_t*)context.esp >= kConsoleRetStart &&
            *(uintptr_t*)context.esp <= kConsoleRetEnd;

        std::uint8_t clear_seed_effects = 0;
        std::uint8_t progress_scared_heart = 0;
        if (!mem::safe_read_raw(
                &clear_seed_effects, context.esp + kOffsetRestartClearSeedEffects, sizeof(clear_seed_effects)) ||
            !mem::safe_read_raw(
                &progress_scared_heart, context.esp + kOffsetRestartProgressScaredHeart, sizeof(progress_scared_heart))) {
            return;
        }

        RestartGameContext ctx{clear_seed_effects != 0, progress_scared_heart != 0};
        ctx.from_console = from_console;

        log_().debug("[RestartGame] clear_seed_effects={} progress_scared_heart={} from_console={}",
                     ctx.clear_seed_effects ? 1 : 0, ctx.progress_scared_heart ? 1 : 0,
                     ctx.from_console ? 1 : 0);

        emit(ctx);
    }
}
