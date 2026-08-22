// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/event/hurt_handler.h"

namespace app::rule
{
    void HurtHandler::on_event(const event::EventVariant& value, const std::chrono::steady_clock::time_point now)
    {
        const auto* hurt = std::get_if<event::HurtEvent>(&value);
        if (!hurt) return;
        process(hurt->context, value, now);
    }

    bool HurtHandler::extra_match(const CompiledEventRule& rule, const event::EventVariant& value) const
    {
        const auto* hurt = std::get_if<event::HurtEvent>(&value);
        if (!hurt || !hurt->details.entity_ref) return true;
        const auto* hurt_rule = std::get_if<CompiledHurtRule>(&rule.event);
        if (!hurt_rule) return true;

        const auto& filter = hurt_rule->entity_filter;
        const auto entity = hurt->details.entity_ref->get_entity();
        const auto spawner = entity.get_spawner();

        if (entity_blacklisted(filter, entity) || entity_blacklisted(filter, spawner)) return false;
        if (!filter.whitelist) return true;
        return entity_whitelisted(filter, entity) || entity_whitelisted(filter, spawner);
    }
}
