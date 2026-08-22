//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_GAME_OVER_H
#define ISAACCOYOTE_GAME_OVER_H

#include <string_view>

#include <nlohmann/json.hpp>

#include "app/event/event_kind.h"
#include "app/rule/types/compiled_filter.h"
#include "app/service/config/player_filter.h"

namespace app::config
{
    struct OnGameOverConfig {
        PlayerFilterConfig players;
    };

    void to_json(nlohmann::json& j, const OnGameOverConfig& v);
    void from_json(const nlohmann::json& j, OnGameOverConfig& v);
}

namespace app::event
{
    struct GameOverEvent {};
}

namespace app::rule
{
    struct CompiledGameOverRule {};

    class GameOverHandler;
}

namespace app::event::sources
{
    class GameOverSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::GameOver> {
        using Config = config::OnGameOverConfig;
        using Event = event::GameOverEvent;
        using Compiled = rule::CompiledGameOverRule;
        using Handler = rule::GameOverHandler;
        using Source = sources::GameOverSource;

        static constexpr std::string_view json_tag = "OnGameOver";
        static constexpr std::string_view ui_name = "GameOver";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_GAME_OVER_H
