// Created by TsCat on 2026/8/10.

#include "isaac_spy/hooks/use_pill_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Entity_Player::UsePill";
        constexpr int kColorIdMask = 0x7FF;
        constexpr int kGoldenBit = 0xB;

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    UsePillHook::UsePillHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternEntityPlayerUsePill, kOffsetUsePillHook},
                   &trampoline, &s_active) {}

    void UsePillHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<UsePillHook*>(hook)->handle(context);
    }

    void UsePillHook::handle(SafetyHookContext& context) {
        UsePillContext use{};
        use.player = context.edi;

        // EBX = entry ESP (MOV EBX,ESP in prologue), stable for the whole function:
        // [EBX+0x8]=PillEffect, [EBX+0xC]=PillColor, [EBX+0x10]=useFlag.
        bool read_ok =
                mem::safe_read_raw(&use.pill_effect, context.ebx + kOffsetUsePillPillEffect, sizeof(use.pill_effect)) &&
                mem::safe_read_raw(&use.pill_color, context.ebx + kOffsetUsePillPillColor, sizeof(use.pill_color)) &&
                mem::safe_read_raw(&use.use_flags, context.ebx + kOffsetUsePillUseFlags, sizeof(use.use_flags));
        if (!read_ok) return;

        use.color_id = use.pill_color & kColorIdMask;
        use.golden = (use.pill_color >> kGoldenBit) & 1;

        log_().debug("[UsePill] player={:#x} effect={} color={:#x} colorId={} golden={} flags={:#x}",
                     use.player, use.pill_effect, use.pill_color, use.color_id, use.golden ? 1 : 0, use.use_flags);

        emit(use);
    }
}
