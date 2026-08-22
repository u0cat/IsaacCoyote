//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_USE_ACTIVE_ITEM_H
#define ISAACCOYOTE_USE_ACTIVE_ITEM_H

#include <optional>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "app/event/context.h"
#include "app/event/event_kind.h"
#include "app/rule/types/compiled_filter.h"
#include "app/service/config/player_filter.h"
#include "isaac_spy/hooks/types.h"

namespace app::config
{
    struct OnUseActiveItemConfig {
        PlayerFilterConfig players;

        bool whitelist = false;
        std::vector<int> whitelist_items;
        std::vector<int> blacklist_items;
    };

    void to_json(nlohmann::json& j, const OnUseActiveItemConfig& v);
    void from_json(const nlohmann::json& j, OnUseActiveItemConfig& v);
}

namespace app::event
{
    struct UseActiveItemEvent {
        std::optional<EventContext> context;
        isaac_spy::hook::UseActiveItemContext details;
    };
}

namespace app::rule
{
    struct CompiledUseActiveItemRule {
        CompiledItemFilter item_filter;
    };

    class UseActiveItemHandler;
}

namespace app::event::sources
{
    class UseActiveItemSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::UseActiveItem> {
        using Config = config::OnUseActiveItemConfig;
        using Event = event::UseActiveItemEvent;
        using Compiled = rule::CompiledUseActiveItemRule;
        using Handler = rule::UseActiveItemHandler;
        using Source = sources::UseActiveItemSource;

        static constexpr std::string_view json_tag = "OnUseActiveItem";
        static constexpr std::string_view ui_name = "使用主动道具";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_USE_ACTIVE_ITEM_H