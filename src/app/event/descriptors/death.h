//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_DEATH_H
#define ISAACCOYOTE_DEATH_H

#include <optional>
#include <string_view>

#include <nlohmann/json.hpp>

#include "app/event/context.h"
#include "app/event/event_kind.h"
#include "app/rule/types/compiled_filter.h"
#include "app/service/config/player_filter.h"

namespace app::config
{
    struct OnDeathConfig {
        PlayerFilterConfig players;
    };

    void to_json(nlohmann::json& j, const OnDeathConfig& v);
    void from_json(const nlohmann::json& j, OnDeathConfig& v);
}

namespace app::event
{
    struct DeathEvent {
        std::optional<EventContext> context;
    };
}

namespace app::rule
{
    struct CompiledDeathRule {};

    class DeathHandler;
}

namespace app::event::sources
{
    class PlayerDeathSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::Death> {
        using Config = config::OnDeathConfig;
        using Event = event::DeathEvent;
        using Compiled = rule::CompiledDeathRule;
        using Handler = rule::DeathHandler;
        using Source = sources::PlayerDeathSource;

        static constexpr std::string_view json_tag = "OnDeath";
        static constexpr std::string_view ui_name = "玩家死亡";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_DEATH_H
