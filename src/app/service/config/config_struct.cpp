#include "app/service/config/config_struct.h"

#include <type_traits>
#include "embedded_config.h"

namespace app::config
{

NLOHMANN_JSON_SERIALIZE_ENUM(StrengthModifier, {
    {StrengthModifier::Override, "Override"},
    {StrengthModifier::Increase, "Increase"},
    {StrengthModifier::Decrease, "Decrease"},
    {StrengthModifier::Multiply, "Multiply"},
})

void to_json(nlohmann::json& j, const ModifierConfig& v)
{
    j = nlohmann::json::object();
    j["modifier"] = v.modifier;
    j["value"] = v.value;
}

void from_json(const nlohmann::json& j, ModifierConfig& v)
{
    j.at("modifier").get_to(v.modifier);
    j.at("value").get_to(v.value);
}

void to_json(nlohmann::json& j, const ChannelModifiersConfig& v)
{
    j = nlohmann::json{{"channel_a", v.channel_a}, {"channel_b", v.channel_b}};
}

void from_json(const nlohmann::json& j, ChannelModifiersConfig& v)
{
    if (auto channel = j.find("channel_a"); channel != j.end() && !channel->is_null())
        v.channel_a = channel->get<ModifierConfig>();
    else
        v.channel_a.reset();
    if (auto channel = j.find("channel_b"); channel != j.end() && !channel->is_null())
        v.channel_b = channel->get<ModifierConfig>();
    else
        v.channel_b.reset();
}

void to_json(nlohmann::json& j, const StrengthEffectConfig& v)
{
    j = nlohmann::json::object();
    j["temporary"] = v.temporary;
    if (v.duration.has_value())
        j["duration"] = v.duration->count();
    j["modifiers"] = v.modifiers;
}

void from_json(const nlohmann::json& j, StrengthEffectConfig& v)
{
    j.at("temporary").get_to(v.temporary);
    auto duration = j.find("duration");
    if (duration != j.end() && !duration->is_null())
        v.duration = Duration(duration->get<int64_t>());
    else
        v.duration.reset();
    j.at("modifiers").get_to(v.modifiers);
}

void to_json(nlohmann::json& j, const PulseEffectConfig& v)
{
    j = nlohmann::json::object();
    j["duration"] = v.duration.count();
    j["pulse_a"] = v.pulse_a;
    j["pulse_b"] = v.pulse_b;

    j["shake"] = v.shake;
    j["shake_duration"] = v.shake_duration.count();
}

void from_json(const nlohmann::json& j, PulseEffectConfig& v)
{
    v.duration = Duration(j.at("duration").get<int64_t>());
    j.at("pulse_a").get_to(v.pulse_a);
    j.at("pulse_b").get_to(v.pulse_b);

    v.shake = j.value("shake", false);
    v.shake_duration = Duration(j.value("shake_duration", int64_t{0}));
}

void to_json(nlohmann::json& j, const EventAction& v)
{
    if (std::holds_alternative<PulseEffectConfig>(v))
    {
        j = std::get<PulseEffectConfig>(v);
        j["type"] = "pulse";
    }
    else if (std::holds_alternative<StrengthEffectConfig>(v))
    {
        const auto& s = std::get<StrengthEffectConfig>(v);
        to_json(j, s);
        j["type"] = "strength";
    }
}

void from_json(const nlohmann::json& j, EventAction& v)
{
    std::string type;
    j.at("type").get_to(type);
    if (type == "pulse")
    {
        v = j.get<PulseEffectConfig>();
    }
    else
    {
        if (type != "strength")
            throw nlohmann::json::other_error::create(501, "unknown event action type: " + type, &j);
        v = j.get<StrengthEffectConfig>();
    }
}

// void to_json(nlohmann::json& j, const WsConfig& v)
// {
//     j = nlohmann::json::object();
//     j["host"] = v.host;
//     j["port"] = v.port;
// }
//
// void from_json(const nlohmann::json& j, WsConfig& v)
// {
//     j.at("host").get_to(v.host);
//     j.at("port").get_to(v.port);
// }

void to_json(nlohmann::json& j, const EventRule& v)
{
    j = nlohmann::json::object();
    j["rule_id"] = v.rule_id;
    j["name"] = v.name.empty() ? v.rule_id : v.name;
    j["enabled"] = v.enabled;
    j["event"] = v.event;
    j["actions"] = v.actions;
}

void from_json(const nlohmann::json& j, EventRule& v)
{
    j.at("rule_id").get_to(v.rule_id);
    if (const auto it = j.find("name"); it != j.end())
        it->get_to(v.name);
    if (v.name.empty()) v.name = v.rule_id;
    j.at("enabled").get_to(v.enabled);
    j.at("event").get_to(v.event);
    j.at("actions").get_to(v.actions);
}

void to_json(nlohmann::json& j, const StrengthConfig& v)
{
    j = nlohmann::json::object();
    j["base_a"] = v.base_a;
    j["base_b"] = v.base_b;
    j["limit_a"] = v.limit_a;
    j["limit_b"] = v.limit_b;
}

void from_json(const nlohmann::json& j, StrengthConfig& v)
{
    j.at("base_a").get_to(v.base_a);
    j.at("base_b").get_to(v.base_b);
    j.at("limit_a").get_to(v.limit_a);
    j.at("limit_b").get_to(v.limit_b);
}

void to_json(nlohmann::json& j, const DecayConfig& v)
{
    j = nlohmann::json::object();
    j["enabled"] = v.enabled;
    j["interval"] = v.interval.count();
    j["amount"] = v.amount;
}

void from_json(const nlohmann::json& j, DecayConfig& v)
{
    j.at("enabled").get_to(v.enabled);
    v.interval = Duration(j.at("interval").get<int64_t>());
    j.at("amount").get_to(v.amount);
}

void to_json(nlohmann::json& j, const ClimbConfig& v)
{
    j = nlohmann::json::object();
    j["enabled"] = v.enabled;
    j["interval"] = v.interval.count();
    j["amount"] = v.amount;
}

void from_json(const nlohmann::json& j, ClimbConfig& v)
{
    j.at("enabled").get_to(v.enabled);
    v.interval = Duration(j.at("interval").get<int64_t>());
    j.at("amount").get_to(v.amount);
}

void to_json(nlohmann::json& j, const ConstantModeConfig& v)
{
    j = nlohmann::json::object();
    j["enabled"] = v.enabled;
    j["pulse_a"] = v.pulse_a;
    j["pulse_b"] = v.pulse_b;
}

void from_json(const nlohmann::json& j, ConstantModeConfig& v)
{
    j.at("enabled").get_to(v.enabled);
    j.at("pulse_a").get_to(v.pulse_a);
    j.at("pulse_b").get_to(v.pulse_b);
}

void to_json(nlohmann::json& j, const CollectibleSourceConfig& v)
{
    j = nlohmann::json::object();
    if (!v.modifiers_by_quality.empty())
    {
        auto& obj = j["modifiers_by_quality"] = nlohmann::json::object();
        for (const auto& [id, effect] : v.modifiers_by_quality)
            obj[std::to_string(id)] = effect;
    }
    if (!v.override_rule.empty())
    {
        auto& obj = j["override_rule"] = nlohmann::json::object();
        for (const auto& [id, effect] : v.override_rule)
            obj[std::to_string(id)] = effect;
    }
}

void from_json(const nlohmann::json& j, CollectibleSourceConfig& v)
{
    if (j.contains("modifiers_by_quality"))
    {
        v.modifiers_by_quality.clear();
        for (auto it = j["modifiers_by_quality"].begin(); it != j["modifiers_by_quality"].end(); ++it)
        {
            int id = std::stoi(it.key());
            v.modifiers_by_quality[id] = it->get<ChannelModifiersConfig>();
        }
    }
    if (j.contains("override_rule"))
    {
        v.override_rule.clear();
        for (auto it = j["override_rule"].begin(); it != j["override_rule"].end(); ++it)
        {
            int id = std::stoi(it.key());
            v.override_rule[id] = it->get<ChannelModifiersConfig>();
        }
    }
}

void to_json(nlohmann::json& j, const HealthSourceConfig& v)
{
    j = nlohmann::json::object();
    j["per_red_heart"] = v.per_red_heart;
}

void from_json(const nlohmann::json& j, HealthSourceConfig& v)
{
    j.at("per_red_heart").get_to(v.per_red_heart);
}

void to_json(nlohmann::json& j, const StaticSourceConfig& v)
{
    if (std::holds_alternative<CollectibleSourceConfig>(v))
    {
        j = std::get<CollectibleSourceConfig>(v);
        j["type"] = "collectible";
    }
    else
    {
        j = std::get<HealthSourceConfig>(v);
        j["type"] = "health";
    }
}

void from_json(const nlohmann::json& j, StaticSourceConfig& v)
{
    const auto type = j.at("type").get<std::string>();
    if (type == "collectible")
    {
        auto jc = j;
        jc.erase("type");
        v = jc.get<CollectibleSourceConfig>();
    }
    else if (type == "health")
    {
        v = j.get<HealthSourceConfig>();
    }
    else
        throw nlohmann::json::other_error::create(501, "unknown static source type: " + type, &j);
}

void to_json(nlohmann::json& j, const StaticRuleConfig& v)
{
    j = nlohmann::json{
        {"rule_id", v.rule_id},
        {"name", v.name.empty() ? v.rule_id : v.name},
        {"enabled", v.enabled},
        {"players", v.players},
        {"source", v.source},
    };
}

void from_json(const nlohmann::json& j, StaticRuleConfig& v)
{
    j.at("rule_id").get_to(v.rule_id);
    if (const auto it = j.find("name"); it != j.end())
        it->get_to(v.name);
    if (v.name.empty()) v.name = v.rule_id;
    j.at("enabled").get_to(v.enabled);
    j.at("players").get_to(v.players);
    j.at("source").get_to(v.source);
}

void to_json(nlohmann::json& j, const GameConfig& v)
{
    j = nlohmann::json::object();
    j["strength"] = v.strength;
    j["climb"] = v.climb;
    j["decay"] = v.decay;
    j["constant_mode"] = v.constant_mode;
    j["static_rules"] = v.static_rules;
    j["events"] = v.events;
}

void from_json(const nlohmann::json& j, GameConfig& v)
{
    j.at("strength").get_to(v.strength);
    j.at("climb").get_to(v.climb);
    j.at("decay").get_to(v.decay);
    j.at("constant_mode").get_to(v.constant_mode);
    j.at("static_rules").get_to(v.static_rules);
    j.at("events").get_to(v.events);
}

void to_json(nlohmann::json& j, const OverlayConfig& v)
{
    j = nlohmann::json{
        {"automatic_dpi", v.automatic_dpi},
        {"scale", v.scale},
        {"menu_key", v.menu_key},
    };
}

void from_json(const nlohmann::json& j, OverlayConfig& v)
{
    if (const auto it = j.find("automatic_dpi"); it != j.end()) it->get_to(v.automatic_dpi);
    if (const auto it = j.find("scale"); it != j.end()) it->get_to(v.scale);
    if (const auto it = j.find("menu_key"); it != j.end()) it->get_to(v.menu_key);

    // F2 and End -> fallback
    if (v.menu_key <= 0 || v.menu_key > 0xFF || v.menu_key == 0x71 || v.menu_key == 0x23)
        v.menu_key = 0x2D;
}

void to_json(nlohmann::json& j, const LoggingConfig& v)
{
    j = nlohmann::json::object();
    j["level"] = v.level;
    j["loggers"] = v.loggers;
}

void from_json(const nlohmann::json& j, LoggingConfig& v)
{
    if (const auto it = j.find("level"); it != j.end()) it->get_to(v.level);
    if (const auto it = j.find("loggers"); it != j.end()) it->get_to(v.loggers);
}

void to_json(nlohmann::json& j, const AppConfig& v)
{
    j = nlohmann::json::object();
    j["version"] = v.version;
    j["ws_endpoint"] = v.ws_endpoint;
    j["pulse_definitions"] = v.pulse_definitions;
    j["game"] = v.game;
    j["overlay"] = v.overlay;
    j["logging"] = v.logging;
}

void from_json(const nlohmann::json& j, AppConfig& v)
{
    j.at("version").get_to(v.version);
    j.at("ws_endpoint").get_to(v.ws_endpoint);
    auto it = j.find("pulse_definitions");
    if (it != j.end()) it->get_to(v.pulse_definitions);
    j.at("game").get_to(v.game);
    if (const auto overlay = j.find("overlay"); overlay != j.end()) overlay->get_to(v.overlay);
    if (const auto logging = j.find("logging"); logging != j.end()) logging->get_to(v.logging);
}

const AppConfig kDefaultConfig = []() -> AppConfig {
    try {
        // kEmbeddedConfigJson is generated at build time from isaac-coyote.json.
        // The file may contain UTF-8 BOM (EF BB BF) after the raw string's leading
        // newline. nlohmann::json does not skip BOM, so remove it explicitly.
        std::string tmp(kEmbeddedConfigJson);
        if (auto pos = tmp.find("\xEF\xBB\xBF"); pos != std::string::npos) {
            tmp.erase(pos, 3);
        } else if (tmp.size() >= 3 && static_cast<unsigned char>(tmp[0]) == 0xEF) {
            // Fallback: BOM at very start
            tmp.erase(0, 3);
        }
        auto j = nlohmann::json::parse(tmp);
        AppConfig cfg = j.get<AppConfig>();
        // Ensure version field tracks kConfigVersion even if JSON drifts.
        // The embedded JSON is canonical, so normally cfg.version == kConfigVersion.
        return cfg;
    } catch (const std::exception&) {
        // Fallback to minimal valid config if parsing fails — avoids static init crash.
        AppConfig fallback;
        fallback.version = kConfigVersion;
        return fallback;
    }
}();

} // namespace config
