//
// Created by TsCat on 2026/7/26.
//

#ifndef ISAACCOYOTE_PLAYER_DEATH_SOURCE_H
#define ISAACCOYOTE_PLAYER_DEATH_SOURCE_H

#include "app/event/descriptors/death.h"
#include "app/event/sources/hook_source.h"
#include "isaac_spy/hooks/player_death_hook.h"

namespace app::event::sources
{
    class PlayerDeathSource final
        : public HookSource<isaac_spy::hook::PlayerDeathContext, DeathEvent, isaac_spy::hook::PlayerDeathHook> {
    public:
        PlayerDeathSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

    protected:
        DeathEvent make_event(isaac_spy::hook::PlayerDeathContext&& ctx) override;
        void log_event(const isaac_spy::hook::PlayerDeathContext& ctx, event::PlayerRelation rel) override;
    };
}

#endif //ISAACCOYOTE_PLAYER_DEATH_SOURCE_H