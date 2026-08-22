// Created by TsCat on 2026/8/10.

#include "isaac_spy/hooks/use_active_item_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "Entity_Player::UseActiveItem";

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    UseActiveItemHook::UseActiveItemHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternEntityPlayerUseActiveItem, kOffsetUseActiveItemHook},
                   &trampoline, &s_active) {}

    void UseActiveItemHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<UseActiveItemHook*>(hook)->handle(context);
    }

    void UseActiveItemHook::handle(SafetyHookContext& context) {
        UseActiveItemContext use{};
        use.player = context.ecx;

        const auto entry_esp = context.esp;
        bool read_ok =
                mem::safe_read_raw(&use.result_flags, entry_esp + kOffsetUseActiveItemResultFlags, sizeof(use.result_flags)) &&
                mem::safe_read_raw(&use.item_id, entry_esp + kOffsetUseActiveItemItemId, sizeof(use.item_id)) &&
                mem::safe_read_raw(&use.use_flags, entry_esp + kOffsetUseActiveItemUseFlags, sizeof(use.use_flags)) &&
                mem::safe_read_raw(&use.active_slot, entry_esp + kOffsetUseActiveItemActiveSlot, sizeof(use.active_slot)) &&
                mem::safe_read_raw(&use.var_data, entry_esp + kOffsetUseActiveItemVarData, sizeof(use.var_data));
        if (!read_ok) return;

        log_().debug("[UseActiveItem] player={:#x} item={} flags={:#x} slot={} var={} result_flags={:#x}",
                     use.player, use.item_id, use.use_flags, use.active_slot, use.var_data, use.result_flags);

        emit(use);
    }
}
