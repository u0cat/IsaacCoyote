// Created by TsCat on 2026/8/10.

#ifndef ISAACSPY_USE_PILL_HOOK_H
#define ISAACSPY_USE_PILL_HOOK_H

#include <safetyhook.hpp>

#include "isaac_spy/hooks/hook_base.h"
#include "isaac_spy/hooks/types.h"

namespace isaac_spy::hook
{
    class UsePillHook final : public HookBase {
    public:
        using Subscription = HookBase::Subscription;

        explicit UsePillHook(HookManager& manager);

        void handle(SafetyHookContext& context) override;

    private:
        static void trampoline(SafetyHookContext& context);
        inline static HookBase* s_active = nullptr;
    };
}

#endif // ISAACSPY_USE_PILL_HOOK_H
