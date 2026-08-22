//
// Created by TsCat on 2026/7/16.
//

#ifndef ISAACCOYOTE_USE_ACTIVE_ITEM_SOURCE_H
#define ISAACCOYOTE_USE_ACTIVE_ITEM_SOURCE_H

#include "app/event/descriptors/use_active_item.h"
#include "app/event/sources/hook_source.h"
#include "isaac_spy/hooks/use_active_item_hook.h"

namespace app::event::sources
{
    class UseActiveItemSource final
        : public HookSource<isaac_spy::hook::UseActiveItemContext, UseActiveItemEvent,
                            isaac_spy::hook::UseActiveItemHook> {
    public:
        UseActiveItemSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

    protected:
        bool accept(const isaac_spy::hook::UseActiveItemContext& ctx) override;
        UseActiveItemEvent make_event(isaac_spy::hook::UseActiveItemContext&& ctx) override;
        void log_event(const isaac_spy::hook::UseActiveItemContext& ctx, event::PlayerRelation rel) override;
    };
}

#endif // ISAACCOYOTE_USE_ACTIVE_ITEM_SOURCE_H