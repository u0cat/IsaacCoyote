//
// Created by TsCat on 2026/7/15.
//

#ifndef ISAACCOYOTE_CONFIG_STRUCT_H
#define ISAACCOYOTE_CONFIG_STRUCT_H
#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

#include "app/event/catalog.h"
#include "app/service/config/player_filter.h"

namespace app::config
{
    inline const std::string kConfigVersion = "1";

    enum class StrengthModifier {
        Override,
        Increase,
        Decrease,
        Multiply
    };

    using PulseName = std::string;
    using RuleId = std::string;
    using StrengthValue = double;
    using Duration = std::chrono::milliseconds;

    struct ModifierConfig {
        StrengthModifier modifier = StrengthModifier::Override;
        StrengthValue value = 1.0;
    };

    struct ChannelModifiersConfig {
        std::optional<ModifierConfig> channel_a;
        std::optional<ModifierConfig> channel_b;
    };

    struct StrengthEffectConfig {
        bool temporary = false; // Whether the effect is temporary or permanent
        std::optional<Duration> duration;

        ChannelModifiersConfig modifiers;
    };

    struct PulseEffectConfig {
        Duration duration{0};
        PulseName pulse_a;
        PulseName pulse_b;

        bool shake = false;
        Duration shake_duration{0};
    };

    using EventAction = std::variant<PulseEffectConfig, StrengthEffectConfig>;

    // Event configuration (event::EventConfig) is defined by the event catalog.
    struct EventRule {
        RuleId rule_id;
        std::string name;
        bool enabled = false;

        event::EventConfig event{};
        std::vector<EventAction> actions;
    };

    struct StrengthConfig {
        StrengthValue base_a = 0;
        StrengthValue base_b = 0;
        StrengthValue limit_a = 0;
        StrengthValue limit_b = 0;
    };

    struct DecayConfig {
        bool enabled = false;
        Duration interval{0};
        StrengthValue amount = 0;
    };

    struct ClimbConfig {
        bool enabled = true;
        Duration interval{0};
        StrengthValue amount = 0;
    };

    struct ConstantModeConfig {
        bool enabled = false;
        PulseName pulse_a;
        PulseName pulse_b;
    };

    struct CollectibleSourceConfig {
        std::unordered_map<int, ChannelModifiersConfig> modifiers_by_quality;
        std::unordered_map<int, ChannelModifiersConfig> override_rule;
    };

    struct HealthSourceConfig {
        ChannelModifiersConfig per_red_heart;
    };

    using StaticSourceConfig = std::variant<CollectibleSourceConfig, HealthSourceConfig>;

    struct StaticRuleConfig {
        RuleId rule_id;
        std::string name;
        bool enabled = false;
        PlayerFilterConfig players;
        StaticSourceConfig source;
    };

    struct GameConfig {
        StrengthConfig strength;
        ClimbConfig climb;
        DecayConfig decay;
        ConstantModeConfig constant_mode;

        std::vector<StaticRuleConfig> static_rules;
        std::vector<EventRule> events;
    };

    struct OverlayConfig {
        bool automatic_dpi = true;
        float scale = 1.0f;
        int menu_key = 0x2D; // INSERT
    };

    struct LoggingConfig {
        std::string level = "info";
        std::map<std::string, std::string> loggers;
    };

    using PulseDefinitions = std::unordered_map<PulseName, std::vector<std::string>>;

    struct AppConfig {
        std::string version = kConfigVersion;
        std::string ws_endpoint;
        PulseDefinitions pulse_definitions;
        GameConfig game;
        OverlayConfig overlay;
        LoggingConfig logging;
    };

    void to_json(nlohmann::json& j, const ModifierConfig& v);
    void from_json(const nlohmann::json& j, ModifierConfig& v);
    void to_json(nlohmann::json& j, const ChannelModifiersConfig& v);
    void from_json(const nlohmann::json& j, ChannelModifiersConfig& v);
    void to_json(nlohmann::json& j, const StrengthEffectConfig& v);
    void from_json(const nlohmann::json& j, StrengthEffectConfig& v);
    void to_json(nlohmann::json& j, const PulseEffectConfig& v);
    void from_json(const nlohmann::json& j, PulseEffectConfig& v);
    void to_json(nlohmann::json& j, const EventAction& v);
    void from_json(const nlohmann::json& j, EventAction& v);
    void to_json(nlohmann::json& j, const EventRule& v);
    void from_json(const nlohmann::json& j, EventRule& v);
    void to_json(nlohmann::json& j, const StrengthConfig& v);
    void from_json(const nlohmann::json& j, StrengthConfig& v);
    void to_json(nlohmann::json& j, const DecayConfig& v);
    void from_json(const nlohmann::json& j, DecayConfig& v);
    void to_json(nlohmann::json& j, const ClimbConfig& v);
    void from_json(const nlohmann::json& j, ClimbConfig& v);
    void to_json(nlohmann::json& j, const ConstantModeConfig& v);
    void from_json(const nlohmann::json& j, ConstantModeConfig& v);
    void to_json(nlohmann::json& j, const CollectibleSourceConfig& v);
    void from_json(const nlohmann::json& j, CollectibleSourceConfig& v);
    void to_json(nlohmann::json& j, const HealthSourceConfig& v);
    void from_json(const nlohmann::json& j, HealthSourceConfig& v);
    void to_json(nlohmann::json& j, const StaticSourceConfig& v);
    void from_json(const nlohmann::json& j, StaticSourceConfig& v);
    void to_json(nlohmann::json& j, const StaticRuleConfig& v);
    void from_json(const nlohmann::json& j, StaticRuleConfig& v);
    void to_json(nlohmann::json& j, const GameConfig& v);
    void from_json(const nlohmann::json& j, GameConfig& v);
    void to_json(nlohmann::json& j, const OverlayConfig& v);
    void from_json(const nlohmann::json& j, OverlayConfig& v);
    void to_json(nlohmann::json& j, const LoggingConfig& v);
    void from_json(const nlohmann::json& j, LoggingConfig& v);
    void to_json(nlohmann::json& j, const AppConfig& v);
    void from_json(const nlohmann::json& j, AppConfig& v);

    extern const AppConfig kDefaultConfig;
}

#endif //ISAACCOYOTE_CONFIG_STRUCT_H