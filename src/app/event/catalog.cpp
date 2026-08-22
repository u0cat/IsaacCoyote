//
// Created by TsCat on 2026/8/19.
//

#include "app/event/catalog.h"

namespace app::event
{
    EventType event_kind_of(const EventConfig& event)
    {
        return std::visit([](const auto& config) -> EventType
        {
            using C = std::decay_t<decltype(config)>;
            if constexpr (std::is_same_v<C, NoneEventConfig>) return EventType::None;
            else return config_event_of<C>();
        }, event);
    }

    EventType event_kind_of(const CompiledEventConfig& event)
    {
        return std::visit([](const auto& config) { return compiled_event_of<std::decay_t<decltype(config)>>(); }, event);
    }

    EventType event_type_of(const EventVariant& event)
    {
        return std::visit([](const auto& value) { return event_of<std::decay_t<decltype(value)>>(); }, event);
    }

    EventConfig make_event_config(const EventType type)
    {
        EventConfig result{NoneEventConfig{}};
        for_each_event_type([&]<EventType T>()
        {
            if (T == type) result = typename EventDescriptor<T>::Config{};
        });
        return result;
    }

    CompiledEventConfig compile_event_config(const EventConfig& config)
    {
        CompiledEventConfig result{};
        std::visit([&](const auto& cfg)
        {
            using C = std::decay_t<decltype(cfg)>;
            if constexpr (!std::is_same_v<C, NoneEventConfig>)
                result = EventDescriptor<config_event_of<C>()>::compile(cfg);
        }, config);
        return result;
    }

    void to_json(nlohmann::json& j, const EventConfig& v)
    {
        std::visit([&j](const auto& config) { to_json(j, config); }, v);
        j["type"] = json_tag_of(event_kind_of(v));
    }

    void from_json(const nlohmann::json& j, EventConfig& v)
    {
        const auto tag = j.at("type").get<std::string>();
        const auto kind = event_type_of_json_tag(tag);
        if (!kind) {
            throw nlohmann::json::other_error::create(501, "unknown event type: " + tag, &j);
        }
        v = make_event_config(*kind);
        std::visit([&](auto& cfg)
        {
            using C = std::decay_t<decltype(cfg)>;
            if constexpr (!std::is_same_v<C, NoneEventConfig>)
                cfg = j.get<C>();
        }, v);
    }

    void to_json(nlohmann::json& j, const NoneEventConfig&)
    {
        j = nlohmann::json::object();
    }

    void from_json(const nlohmann::json& j, NoneEventConfig&)
    {
        (void)j;
    }
}
