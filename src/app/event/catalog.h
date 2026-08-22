//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_CATALOG_H
#define ISAACCOYOTE_CATALOG_H

#include <array>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include <nlohmann/json.hpp>

#include "app/event/descriptors/death.h"
#include "app/event/descriptors/game_over.h"
#include "app/event/descriptors/hurt.h"
#include "app/event/descriptors/reroll.h"
#include "app/event/descriptors/use_active_item.h"
#include "app/event/descriptors/use_card.h"
#include "app/event/descriptors/use_pill.h"
#include "app/event/event_kind.h"

namespace app::event
{
    template <typename F>
    constexpr void for_each_event_type(F&& f)
    {
        AllEventTypes::for_each(std::forward<F>(f));
    }

    // Variants generated from the descriptor list
    template <typename L>
    struct EventVariantFromList;

    template <EventType... Ts>
    struct EventVariantFromList<EventTypeList<Ts...>> {
        using type = std::variant<typename EventDescriptor<Ts>::Event...>;
    };

    using EventVariant = typename EventVariantFromList<AllEventTypes>::type;

    template <typename L>
    struct EventConfigFromList;

    template <EventType... Ts>
    struct EventConfigFromList<EventTypeList<Ts...>> {
        using type = std::variant<typename EventDescriptor<Ts>::Config..., NoneEventConfig>;
    };

    using EventConfig = typename EventConfigFromList<AllEventTypes>::type;

    template <typename L>
    struct CompiledConfigFromList;

    template <EventType... Ts>
    struct CompiledConfigFromList<EventTypeList<Ts...>> {
        using type = std::variant<typename EventDescriptor<Ts>::Compiled...>;
    };

    using CompiledEventConfig = typename CompiledConfigFromList<AllEventTypes>::type;

    // Reverse lookups: member type -> EventType
    template <typename ConfigT>
    constexpr EventType config_event_of()
    {
        EventType result = EventType::None;
        for_each_event_type([&]<EventType T>()
        {
            if constexpr (std::is_same_v<ConfigT, typename EventDescriptor<T>::Config>) result = T;
        });
        return result;
    }

    template <typename EventT>
    constexpr EventType event_of()
    {
        EventType result = EventType::None;
        for_each_event_type([&]<EventType T>()
        {
            if constexpr (std::is_same_v<EventT, typename EventDescriptor<T>::Event>) result = T;
        });
        return result;
    }

    template <typename CompiledT>
    constexpr EventType compiled_event_of()
    {
        EventType result = EventType::None;
        for_each_event_type([&]<EventType T>()
        {
            if constexpr (std::is_same_v<CompiledT, typename EventDescriptor<T>::Compiled>) result = T;
        });
        return result;
    }

    EventType event_kind_of(const EventConfig& event);

    EventType event_kind_of(const CompiledEventConfig& event);

    EventType event_type_of(const EventVariant& event);

    // Catalog metadata: json tag + UI name per event
    struct EventMeta {
        EventType type;
        std::string_view json_tag;
        std::string_view ui_name;
    };

    template <EventType... Ts>
    constexpr std::array<EventMeta, sizeof...(Ts) + 1> collect_catalog(EventTypeList<Ts...>)
    {
        return {
            EventMeta{Ts, EventDescriptor<Ts>::json_tag, EventDescriptor<Ts>::ui_name}...,
            EventMeta{EventType::None, "None", "无"},
        };
    }

    inline constexpr auto kEventCatalog = collect_catalog(AllEventTypes{});

    constexpr std::string_view json_tag_of(EventType type)
    {
        for (const auto& meta : kEventCatalog)
            if (meta.type == type) return meta.json_tag;
        return {};
    }

    constexpr std::optional<EventType> event_type_of_json_tag(std::string_view tag)
    {
        for (const auto& meta : kEventCatalog)
            if (meta.json_tag == tag) return meta.type;
        return std::nullopt;
    }

    constexpr std::string_view ui_name_of(EventType type)
    {
        for (const auto& meta : kEventCatalog)
            if (meta.type == type) return meta.ui_name;
        return {};
    }

    // Construction and compilation dispatch
    EventConfig make_event_config(EventType type);

    CompiledEventConfig compile_event_config(const EventConfig& config);

    void to_json(nlohmann::json& j, const EventConfig& v);
    void from_json(const nlohmann::json& j, EventConfig& v);
    void to_json(nlohmann::json& j, const NoneEventConfig& v);
    void from_json(const nlohmann::json& j, NoneEventConfig& v);
}

#endif //ISAACCOYOTE_CATALOG_H