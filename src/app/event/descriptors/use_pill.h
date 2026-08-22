//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_USE_PILL_H
#define ISAACCOYOTE_USE_PILL_H

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
    struct OnUsePillConfig {
        PlayerFilterConfig players;

        bool whitelist = false;
        std::vector<int> whitelist_pills;
        std::vector<int> blacklist_pills;
    };

    void to_json(nlohmann::json& j, const OnUsePillConfig& v);
    void from_json(const nlohmann::json& j, OnUsePillConfig& v);
}

namespace app::event
{
    struct UsePillEvent {
        std::optional<EventContext> context;
        isaac_spy::hook::UsePillContext details;
    };
}

namespace app::rule
{
    struct CompiledUsePillRule {
        CompiledItemFilter item_filter;
    };

    class UsePillHandler;
}

namespace app::event::sources
{
    class UsePillSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::UsePill> {
        using Config = config::OnUsePillConfig;
        using Event = event::UsePillEvent;
        using Compiled = rule::CompiledUsePillRule;
        using Handler = rule::UsePillHandler;
        using Source = sources::UsePillSource;

        static constexpr std::string_view json_tag = "OnUsePill";
        static constexpr std::string_view ui_name = "使用胶囊";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_USE_PILL_H