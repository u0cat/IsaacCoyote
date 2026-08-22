// Created by TsCat on 2026/7/25.

#ifndef ISAACCOYOTE_START_NEW_GAME_HOOK_H
#define ISAACCOYOTE_START_NEW_GAME_HOOK_H

#include <safetyhook.hpp>

#include "isaac_spy/hooks/hook_base.h"
#include "isaac_spy/hooks/types.h"

namespace isaac_spy::hook
{
    class StartNewGameHook final : public HookBase {
    public:
        using Subscription = HookBase::Subscription;

        explicit StartNewGameHook(HookManager& manager);

        void handle(SafetyHookContext& context) override;

    private:
        static void trampoline(SafetyHookContext& context);
        inline static HookBase* s_active = nullptr;
    };
}

#endif // ISAACCOYOTE_START_NEW_GAME_HOOK_H
