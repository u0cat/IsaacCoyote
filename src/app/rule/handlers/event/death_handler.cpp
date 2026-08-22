// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/event/death_handler.h"

namespace app::rule
{
    void DeathHandler::on_event(const event::EventVariant& value, const std::chrono::steady_clock::time_point now)
    {
        const auto* death = std::get_if<event::DeathEvent>(&value);
        if (!death) return;
        process(death->context, value, now);
    }
}