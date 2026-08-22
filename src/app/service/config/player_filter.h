//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_CONFIG_PLAYER_FILTER_H
#define ISAACCOYOTE_CONFIG_PLAYER_FILTER_H

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

// Leaf config type shared by event rules (per-event descriptors) and static rules.
namespace app::config
{
    enum class PlayerScope {
        Self,
        Others,
        Any,
        Specific
    };

    struct PlayerFilterConfig {
        PlayerScope scope = PlayerScope::Any;
        std::vector<std::string> player_ids;
    };

    void to_json(nlohmann::json& j, const PlayerFilterConfig& v);
    void from_json(const nlohmann::json& j, PlayerFilterConfig& v);
}

#endif //ISAACCOYOTE_CONFIG_PLAYER_FILTER_H
