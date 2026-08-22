// Created by TsCat on 2026/8/10.

#include "app/rule/handlers/event/use_card_handler.h"

namespace app::rule
{
    void UseCardHandler::on_event(const event::EventVariant& value, const std::chrono::steady_clock::time_point now)
    {
        const auto* use = std::get_if<event::UseCardEvent>(&value);
        if (!use) return;
        process(use->context, value, now);
    }

    bool UseCardHandler::extra_match(const CompiledEventRule& rule, const event::EventVariant& value) const
    {
        const auto* use = std::get_if<event::UseCardEvent>(&value);
        if (!use) return true;
        const auto* use_rule = std::get_if<CompiledUseCardRule>(&rule.event);
        if (!use_rule) return true;

        const auto& filter = use_rule->item_filter;
        if (item_blacklisted(filter, use->details.card_id)) return false;
        if (!filter.whitelist) return true;
        return item_whitelisted(filter, use->details.card_id);
    }
}
