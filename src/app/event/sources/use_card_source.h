//
// Created by TsCat on 2026/8/10.
//

#ifndef ISAACCOYOTE_USE_CARD_SOURCE_H
#define ISAACCOYOTE_USE_CARD_SOURCE_H

#include "app/event/descriptors/use_card.h"
#include "app/event/sources/hook_source.h"
#include "isaac_spy/hooks/use_card_hook.h"

namespace app::event::sources
{
    class UseCardSource final
        : public HookSource<isaac_spy::hook::UseCardContext, UseCardEvent, isaac_spy::hook::UseCardHook> {
    public:
        UseCardSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

    protected:
        UseCardEvent make_event(isaac_spy::hook::UseCardContext&& ctx) override;
        void log_event(const isaac_spy::hook::UseCardContext& ctx, event::PlayerRelation rel) override;
    };
}

#endif // ISAACCOYOTE_USE_CARD_SOURCE_H