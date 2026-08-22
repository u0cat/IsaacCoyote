//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_HURT_H
#define ISAACCOYOTE_HURT_H

#include <cstdio>
#include <format>
#include <optional>
#include <string>
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
    struct OnHurtConfig {
        struct EntityKey {
            int type;
            int subtype;
            int variant;

            std::string to_string() const {
                return std::format("{}.{}.{}", type, variant, subtype);
            }

            bool from_string(const std::string& value) {
                int t, s, v;
                char extra = '\0';
                if (std::sscanf(value.c_str(), "%d.%d.%d %c", &t, &v, &s, &extra) != 3) {
                    return false;
                }
                type = t;
                variant = v;
                subtype = s;
                return true;
            }
        };

        PlayerFilterConfig players;

        bool whitelist = false;
        std::vector<EntityKey> whitelist_entities;
        std::vector<EntityKey> blacklist_entities;
    };

    void to_json(nlohmann::json& j, const OnHurtConfig::EntityKey& v);
    void from_json(const nlohmann::json& j, OnHurtConfig::EntityKey& v);
    void to_json(nlohmann::json& j, const OnHurtConfig& v);
    void from_json(const nlohmann::json& j, OnHurtConfig& v);
}

namespace app::event
{
    struct HurtEvent {
        std::optional<EventContext> context;
        isaac_spy::hook::HurtContext details;
    };
}

namespace app::rule
{
    struct CompiledHurtRule {
        CompiledEntityFilter entity_filter;
    };

    class HurtHandler;
}

namespace app::event::sources
{
    class HurtSource;
}

namespace app::event
{
    template <>
    struct EventDescriptor<EventType::Hurt> {
        using Config = config::OnHurtConfig;
        using Event = event::HurtEvent;
        using Compiled = rule::CompiledHurtRule;
        using Handler = rule::HurtHandler;
        using Source = sources::HurtSource;

        static constexpr std::string_view json_tag = "OnHurt";
        static constexpr std::string_view ui_name = "受到伤害";

        static Compiled compile(const Config& config);
        static void register_source(EventSourceManager& manager);
    };
}

#endif //ISAACCOYOTE_HURT_H
