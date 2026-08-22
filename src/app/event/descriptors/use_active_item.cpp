//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/use_active_item.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/use_active_item_source.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnUseActiveItemConfig& v)
    {
        j = nlohmann::json{
            {"players", v.players},
            {"whitelist", v.whitelist},
            {"whitelist_items", v.whitelist_items},
            {"blacklist_items", v.blacklist_items},
        };
    }

    void from_json(const nlohmann::json& j, OnUseActiveItemConfig& v)
    {
        j.at("players").get_to(v.players);
        j.at("whitelist").get_to(v.whitelist);
        if (const auto it = j.find("whitelist_items"); it != j.end())
            it->get_to(v.whitelist_items);
        if (const auto it = j.find("blacklist_items"); it != j.end())
            it->get_to(v.blacklist_items);
    }
}

namespace app::event
{
    EventDescriptor<EventType::UseActiveItem>::Compiled EventDescriptor<EventType::UseActiveItem>::compile(const Config& config)
    {
        Compiled result;
        result.item_filter.whitelist = config.whitelist;
        result.item_filter.whitelist_items.insert(config.whitelist_items.begin(), config.whitelist_items.end());
        result.item_filter.blacklist_items.insert(config.blacklist_items.begin(), config.blacklist_items.end());
        return result;
    }

    void EventDescriptor<EventType::UseActiveItem>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::UseActiveItemSource>(EventType::UseActiveItem, manager.get_hook_manager());
    }
}