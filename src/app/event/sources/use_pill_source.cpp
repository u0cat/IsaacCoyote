//
// Created by TsCat on 2026/8/10.
//

#include "app/event/sources/use_pill_source.h"

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
    UsePillSource::UsePillSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
        : HookSource(engine, hook_manager.get_hook<isaac_spy::hook::UsePillHook>(), "[UsePill]") {}

    UsePillEvent UsePillSource::make_event(isaac_spy::hook::UsePillContext&& ctx) {
        UsePillEvent event{};
        event.details = std::move(ctx);
        return event;
    }

    void UsePillSource::log_event(const isaac_spy::hook::UsePillContext& ctx,
                                  const event::PlayerRelation rel) {
        log_().info("[UsePill] rel={} effect={}", relation_name(rel), ctx.pill_effect);
    }
}