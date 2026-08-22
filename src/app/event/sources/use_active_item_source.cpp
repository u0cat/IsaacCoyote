//
// Created by TsCat on 2026/8/10.
//

#include "app/event/sources/use_active_item_source.h"

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
    UseActiveItemSource::UseActiveItemSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
        : HookSource(engine, hook_manager.get_hook<isaac_spy::hook::UseActiveItemHook>(), "[UseActiveItem]") {}

    bool UseActiveItemSource::accept(const isaac_spy::hook::UseActiveItemContext& ctx) {
        // Internal/recursive calls (UseCard/UsePill/TakeDamage) carry activeSlot == -1.
        if (ctx.active_slot >= 0) return true;
        log_().debug("[UseActiveItem] dropped: internal call (slot={})", ctx.active_slot);
        return false;
    }

    UseActiveItemEvent UseActiveItemSource::make_event(isaac_spy::hook::UseActiveItemContext&& ctx) {
        UseActiveItemEvent event{};
        event.details = std::move(ctx);
        return event;
    }

    void UseActiveItemSource::log_event(const isaac_spy::hook::UseActiveItemContext& ctx,
                                        const event::PlayerRelation rel) {
        log_().info("[UseActiveItem] rel={} item={}", relation_name(rel), ctx.item_id);
    }
}