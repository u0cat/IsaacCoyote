//
// Created by TsCat on 2026/8/19.
//

#include "app/service/config/player_filter.h"

namespace app::config
{

NLOHMANN_JSON_SERIALIZE_ENUM(PlayerScope, {
    {PlayerScope::Self, "Self"},
    {PlayerScope::Others, "Others"},
    {PlayerScope::Any, "Any"},
    {PlayerScope::Specific, "Specific"},
})

void to_json(nlohmann::json& j, const PlayerFilterConfig& v)
{
    j = nlohmann::json{{"scope", v.scope}, {"player_ids", v.player_ids}};
}

void from_json(const nlohmann::json& j, PlayerFilterConfig& v)
{
    j.at("scope").get_to(v.scope);
    if (auto ids = j.find("player_ids"); ids != j.end())
        ids->get_to(v.player_ids);
    else
        v.player_ids.clear();
}

} // namespace config
