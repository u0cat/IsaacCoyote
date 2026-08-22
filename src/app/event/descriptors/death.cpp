//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/death.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/player_death.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnDeathConfig& v)
    {
        j = nlohmann::json{{"players", v.players}};
    }

    void from_json(const nlohmann::json& j, OnDeathConfig& v)
    {
        j.at("players").get_to(v.players);
    }
}

namespace app::event
{
    EventDescriptor<EventType::Death>::Compiled EventDescriptor<EventType::Death>::compile(const Config&)
    {
        return {};
    }

    void EventDescriptor<EventType::Death>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::PlayerDeathSource>(EventType::Death, manager.get_hook_manager());
    }
}
