//
// Created by TsCat on 2026/7/16.
//

#ifndef ISAACCOYOTE_HURT_SOURCE_H
#define ISAACCOYOTE_HURT_SOURCE_H

#include "app/event/descriptors/hurt.h"
#include "app/event/sources/hook_source.h"
#include "isaac_spy/hooks/hurt_hook.h"

namespace app::event::sources
{
    class HurtSource final
        : public HookSource<isaac_spy::hook::HurtContext, HurtEvent, isaac_spy::hook::HurtHook> {
    public:
        HurtSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

    protected:
        HurtEvent make_event(isaac_spy::hook::HurtContext&& ctx) override;
        void log_event(const isaac_spy::hook::HurtContext& ctx, event::PlayerRelation rel) override;
    };
}

#endif // ISAACCOYOTE_HURT_SOURCE_H