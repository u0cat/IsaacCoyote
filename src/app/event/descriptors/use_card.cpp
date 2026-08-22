//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/use_card.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/use_card_source.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnUseCardConfig& v)
    {
        j = nlohmann::json{
            {"players", v.players},
            {"whitelist", v.whitelist},
            {"whitelist_cards", v.whitelist_cards},
            {"blacklist_cards", v.blacklist_cards},
        };
    }

    void from_json(const nlohmann::json& j, OnUseCardConfig& v)
    {
        j.at("players").get_to(v.players);
        j.at("whitelist").get_to(v.whitelist);
        if (const auto it = j.find("whitelist_cards"); it != j.end())
            it->get_to(v.whitelist_cards);
        if (const auto it = j.find("blacklist_cards"); it != j.end())
            it->get_to(v.blacklist_cards);
    }
}

namespace app::event
{
    EventDescriptor<EventType::UseCard>::Compiled EventDescriptor<EventType::UseCard>::compile(const Config& config)
    {
        Compiled result;
        result.item_filter.whitelist = config.whitelist;
        result.item_filter.whitelist_items.insert(config.whitelist_cards.begin(), config.whitelist_cards.end());
        result.item_filter.blacklist_items.insert(config.blacklist_cards.begin(), config.blacklist_cards.end());
        return result;
    }

    void EventDescriptor<EventType::UseCard>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::UseCardSource>(EventType::UseCard, manager.get_hook_manager());
    }
}