//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/use_pill.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/use_pill_source.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnUsePillConfig& v)
    {
        j = nlohmann::json{
            {"players", v.players},
            {"whitelist", v.whitelist},
            {"whitelist_pills", v.whitelist_pills},
            {"blacklist_pills", v.blacklist_pills},
        };
    }

    void from_json(const nlohmann::json& j, OnUsePillConfig& v)
    {
        j.at("players").get_to(v.players);
        j.at("whitelist").get_to(v.whitelist);
        if (const auto it = j.find("whitelist_pills"); it != j.end())
            it->get_to(v.whitelist_pills);
        if (const auto it = j.find("blacklist_pills"); it != j.end())
            it->get_to(v.blacklist_pills);
    }
}

namespace app::event
{
    EventDescriptor<EventType::UsePill>::Compiled EventDescriptor<EventType::UsePill>::compile(const Config& config)
    {
        Compiled result;
        result.item_filter.whitelist = config.whitelist;
        result.item_filter.whitelist_items.insert(config.whitelist_pills.begin(), config.whitelist_pills.end());
        result.item_filter.blacklist_items.insert(config.blacklist_pills.begin(), config.blacklist_pills.end());
        return result;
    }

    void EventDescriptor<EventType::UsePill>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::UsePillSource>(EventType::UsePill, manager.get_hook_manager());
    }
}