//
// Created by TsCat on 2026/8/10.
//

#include "app/event/sources/use_card_source.h"

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
    UseCardSource::UseCardSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
        : HookSource(engine, hook_manager.get_hook<isaac_spy::hook::UseCardHook>(), "[UseCard]") {}

    UseCardEvent UseCardSource::make_event(isaac_spy::hook::UseCardContext&& ctx) {
        UseCardEvent event{};
        event.details = std::move(ctx);
        return event;
    }

    void UseCardSource::log_event(const isaac_spy::hook::UseCardContext& ctx,
                                  const event::PlayerRelation rel) {
        log_().info("[UseCard] rel={} card={}", relation_name(rel), ctx.card_id);
    }
}