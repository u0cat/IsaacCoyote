// Created by TsCat on 2026/7/25.

#ifndef ISAACCOYOTE_RESTART_GAME_HOOK_H
#define ISAACCOYOTE_RESTART_GAME_HOOK_H

#include <safetyhook.hpp>

#include "isaac_spy/hooks/hook_base.h"
#include "isaac_spy/hooks/types.h"

namespace isaac_spy::hook
{
    class RestartGameHook final : public HookBase {
    public:
        using Subscription = HookBase::Subscription;

        explicit RestartGameHook(HookManager& manager);

        void handle(SafetyHookContext& context) override;

    protected:
        // Resolves the console restart return range before install.
        void on_first_subscribe() override;

    private:
        static void trampoline(SafetyHookContext& context);
        inline static HookBase* s_active = nullptr;
    };
}

#endif // ISAACCOYOTE_RESTART_GAME_HOOK_H
