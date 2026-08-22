// Created by TsCat on 2026/8/10.

#include "isaac_spy/hooks/use_card_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Entity_Player::UseCard";
        constexpr unsigned int kRecursionFlag = 0x20;

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    UseCardHook::UseCardHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternEntityPlayerUseCard, kOffsetUseCardHook},
                   &trampoline, &s_active) {}

    void UseCardHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<UseCardHook*>(hook)->handle(context);
    }

    void UseCardHook::handle(SafetyHookContext& context) {
        UseCardContext use{};
        use.player = context.edi;

        // EBP frame is stable at the convergence point:
        // [EBP+0xC]=useFlags, [EBP-0x290]=effective card id, [EBP-0x368]=card config, [EBP-0x294]=card slot.
        bool read_ok =
                mem::safe_read_raw(&use.card_id, context.ebp + kOffsetUseCardCardId, sizeof(use.card_id)) &&
                mem::safe_read_raw(&use.use_flags, context.ebp + kOffsetUseCardUseFlags, sizeof(use.use_flags)) &&
                mem::safe_read_raw(&use.card_config, context.ebp + kOffsetUseCardCardConfig, sizeof(use.card_config)) &&
                mem::safe_read_raw(&use.card_slot, context.ebp + kOffsetUseCardCardSlot, sizeof(use.card_slot));
        if (!read_ok) return;

        // Skip the battery recursion inner call (UseCard(card, 0x20)); the outer call fires once.
        if ((use.use_flags & kRecursionFlag) != 0) return;

        log_().debug("[UseCard] player={:#x} card={} flags={:#x} config={:#x} slot={:#x}",
                     use.player, use.card_id, use.use_flags, use.card_config, use.card_slot);

        emit(use);
    }
}
