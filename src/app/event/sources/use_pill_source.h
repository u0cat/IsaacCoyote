//
// Created by TsCat on 2026/8/10.
//

#ifndef ISAACCOYOTE_USE_PILL_SOURCE_H
#define ISAACCOYOTE_USE_PILL_SOURCE_H

#include "app/event/descriptors/use_pill.h"
#include "app/event/sources/hook_source.h"
#include "isaac_spy/hooks/use_pill_hook.h"

namespace app::event::sources
{
    class UsePillSource final
        : public HookSource<isaac_spy::hook::UsePillContext, UsePillEvent, isaac_spy::hook::UsePillHook> {
    public:
        UsePillSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

    protected:
        UsePillEvent make_event(isaac_spy::hook::UsePillContext&& ctx) override;
        void log_event(const isaac_spy::hook::UsePillContext& ctx, event::PlayerRelation rel) override;
    };
}

#endif // ISAACCOYOTE_USE_PILL_SOURCE_H