//
// Created by TsCat on 2026/7/16.
//

#include "app/event/sources/hurt_source.h"

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
    HurtSource::HurtSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
        : HookSource(engine, hook_manager.get_hook<isaac_spy::hook::HurtHook>(), "[Hurt]") {}

    HurtEvent HurtSource::make_event(isaac_spy::hook::HurtContext&& ctx) {
        HurtEvent event{};
        event.details = std::move(ctx);
        return event;
    }

    void HurtSource::log_event(const isaac_spy::hook::HurtContext& ctx, const event::PlayerRelation rel) {
        log_().info("[Hurt] rel={} player={:#x}", relation_name(rel), ctx.player);
    }
}