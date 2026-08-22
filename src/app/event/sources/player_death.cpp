//
// Created by TsCat on 2026/7/26.
//

#include "app/event/sources/player_death.h"

#include "app/event/event_engine.h"
#include "app/service/log/log_service.h"
#include "isaac_spy/isaac/game.h"

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.event");
        return *logger;
    }
}

namespace app::event::sources
{
    PlayerDeathSource::PlayerDeathSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
        : HookSource(engine, hook_manager.get_hook<isaac_spy::hook::PlayerDeathHook>(), "[PlayerDeath]") {}

    DeathEvent PlayerDeathSource::make_event(isaac_spy::hook::PlayerDeathContext&&) {
        return {};
    }

    void PlayerDeathSource::log_event(const isaac_spy::hook::PlayerDeathContext& ctx,
                                      const event::PlayerRelation rel) {
        log_().info("[PlayerDeath] rel={} player={:#x}", relation_name(rel), ctx.player);
    }
}