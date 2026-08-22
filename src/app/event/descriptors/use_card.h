//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_USE_CARD_H
#define ISAACCOYOTE_USE_CARD_H

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
    struct OnUseCardConfig {
        PlayerFilterConfig players;

        bool whitelist = false;
        std::vector<int> whitelist_cards;
        std::vector<int> blacklist_cards;
    };

    void to_json(nlohmann::json& j, const OnUseCardConfig& v);
    void from_json(const nlohmann::json& j, OnUseCardConfig& v);
}

namespace app::event
{
    struct UseCardEvent {
        std::optional<EventContext> context;
        isaac_spy::hook::UseCardContext details;
    };
}

namespace app::rule
{
    struct CompiledUseCardRule {
        CompiledItemFilter item_filter;
    };

    class UseCardHandler;
}

namespace app::event::sources
{
    class UseCardSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::UseCard> {
        using Config = config::OnUseCardConfig;
        using Event = event::UseCardEvent;
        using Compiled = rule::CompiledUseCardRule;
        using Handler = rule::UseCardHandler;
        using Source = sources::UseCardSource;

        static constexpr std::string_view json_tag = "OnUseCard";
        static constexpr std::string_view ui_name = "使用卡片";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_USE_CARD_H