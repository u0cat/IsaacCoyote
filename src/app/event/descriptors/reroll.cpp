//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/reroll.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/reroll.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnRerollGameConfig& v)
    {
        j = nlohmann::json{
            {"players", v.players},
            {"include_rewind", v.include_rewind},
        };
    }

    void from_json(const nlohmann::json& j, OnRerollGameConfig& v)
    {
        j.at("players").get_to(v.players);
        v.include_rewind = j.value("include_rewind", false);
    }
}

namespace app::event
{
    EventDescriptor<EventType::RerollGame>::Compiled EventDescriptor<EventType::RerollGame>::compile(const Config& config)
    {
        return {config.include_rewind};
    }

    void EventDescriptor<EventType::RerollGame>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::RerollSource>(EventType::RerollGame, manager.get_hook_manager());
    }
}
