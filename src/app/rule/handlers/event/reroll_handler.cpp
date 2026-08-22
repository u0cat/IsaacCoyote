// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/event/reroll_handler.h"

namespace app::rule
{
    void RerollGameHandler::on_event(
        const event::EventVariant& value,
        const std::chrono::steady_clock::time_point now
    ) {
        if (!std::holds_alternative<event::RerollGameEvent>(value)) return;
        process(std::nullopt, value, now);
    }

    void RerollGameHandler::reset() {
        // EventRuleHandler::reset();
        trigger_sequence_ = 0;
    }
}
