//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_REROLL_H
#define ISAACCOYOTE_REROLL_H

#include <string_view>

#include <nlohmann/json.hpp>

#include "app/event/event_kind.h"
#include "app/rule/types/compiled_filter.h"
#include "app/service/config/player_filter.h"

namespace app::config
{
    struct OnRerollGameConfig {
        PlayerFilterConfig players;

        bool include_rewind = false;
    };

    void to_json(nlohmann::json& j, const OnRerollGameConfig& v);
    void from_json(const nlohmann::json& j, OnRerollGameConfig& v);
}

namespace app::event
{
    struct RerollGameEvent {};
}

namespace app::rule
{
    struct CompiledRerollGameRule {
        bool include_rewind = true;
    };

    class RerollGameHandler;
}

namespace app::event::sources
{
    class RerollSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::RerollGame> {
        using Config = config::OnRerollGameConfig;
        using Event = event::RerollGameEvent;
        using Compiled = rule::CompiledRerollGameRule;
        using Handler = rule::RerollGameHandler;
        using Source = sources::RerollSource;

        static constexpr std::string_view json_tag = "OnRerollGame";
        static constexpr std::string_view ui_name = "Reroll";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_REROLL_H
