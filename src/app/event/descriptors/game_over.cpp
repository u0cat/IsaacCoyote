//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/game_over.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/game_over.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnGameOverConfig& v)
    {
        j = nlohmann::json{{"players", v.players}};
    }

    void from_json(const nlohmann::json& j, OnGameOverConfig& v)
    {
        j.at("players").get_to(v.players);
    }
}

namespace app::event
{
    EventDescriptor<EventType::GameOver>::Compiled EventDescriptor<EventType::GameOver>::compile(const Config&)
    {
        return {};
    }

    void EventDescriptor<EventType::GameOver>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::GameOverSource>(EventType::GameOver);
    }
}
