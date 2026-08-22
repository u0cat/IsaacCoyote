//
// Created by TsCat on 2026/8/19.
//

#include "app/event/descriptors/hurt.h"

#include "app/event/catalog.h"
#include "app/event/event_source_manager.h"
#include "app/event/sources/hurt_source.h"

namespace app::config
{
    void to_json(nlohmann::json& j, const OnHurtConfig::EntityKey& v)
    {
        j = nlohmann::json{{"type", v.type}, {"subtype", v.subtype}, {"variant", v.variant}};
    }

    void from_json(const nlohmann::json& j, OnHurtConfig::EntityKey& v)
    {
        j.at("type").get_to(v.type);
        j.at("subtype").get_to(v.subtype);
        j.at("variant").get_to(v.variant);
    }

    void to_json(nlohmann::json& j, const OnHurtConfig& v)
    {
        j = nlohmann::json{
            {"players", v.players},
            {"whitelist", v.whitelist},
            {"whitelist_entities", v.whitelist_entities},
            {"blacklist_entities", v.blacklist_entities},
        };
    }

    void from_json(const nlohmann::json& j, OnHurtConfig& v)
    {
        j.at("players").get_to(v.players);
        j.at("whitelist").get_to(v.whitelist);
        if (const auto it = j.find("whitelist_entities"); it != j.end())
            it->get_to(v.whitelist_entities);
        if (const auto it = j.find("blacklist_entities"); it != j.end())
            it->get_to(v.blacklist_entities);
    }
}

namespace app::event
{
    EventDescriptor<EventType::Hurt>::Compiled EventDescriptor<EventType::Hurt>::compile(const Config& config)
    {
        Compiled result;
        result.entity_filter.whitelist = config.whitelist;
        result.entity_filter.whitelist_entities.reserve(config.whitelist_entities.size());
        for (const auto& key : config.whitelist_entities)
            result.entity_filter.whitelist_entities.push_back({key.type, key.subtype, key.variant});
        result.entity_filter.blacklist_entities.reserve(config.blacklist_entities.size());
        for (const auto& key : config.blacklist_entities)
            result.entity_filter.blacklist_entities.push_back({key.type, key.subtype, key.variant});
        return result;
    }

    void EventDescriptor<EventType::Hurt>::register_source(EventSourceManager& manager)
    {
        manager.register_source<sources::HurtSource>(EventType::Hurt, manager.get_hook_manager());
    }
}
