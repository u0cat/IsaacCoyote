// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/event/game_over_handler.h"

namespace app::rule
{
    void GameOverHandler::on_event(const event::EventVariant& value, const std::chrono::steady_clock::time_point now)
    {
        if (!std::holds_alternative<event::GameOverEvent>(value)) return;
        process_global(value, now);
    }

    void GameOverHandler::reset() {
        // EventRuleHandler::reset();
        trigger_sequence_ = 0;
    }
}
