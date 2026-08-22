//
// Created by TsCat on 2026/7/17.
//

#ifndef ISAACCOYOTE_RULE_TYPE_H
#define ISAACCOYOTE_RULE_TYPE_H
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "app/event/context.h"
#include "app/service/coyote/pulse_helper/types.h"

// Basic
namespace app::rule
{
    using RuleId = std::string;
    using RuleOrder = std::uint32_t;
    using TriggerSequence = std::uint64_t;
    using ActionOrder = std::uint32_t;

    // Event context lives in the event layer; re-exported for rule-side call sites.
    using PlayerId = app::event::PlayerId;
    using PlayerRelation = app::event::PlayerRelation;
    using EventContext = app::event::EventContext;
    using app::event::ptr_player_id;
}

// Rule condition
namespace app::rule
{
    enum class PlayerScope {
        Self,
        Others,
        Any,
    };
}

// Modifier
namespace app::rule
{
    enum class ModifierOperation {
        Override,
        Increase,
        Decrease,
        Multiply,
    };

    struct Modifier {
        ModifierOperation operation = ModifierOperation::Override;
        double value = 0.0;

        bool operator==(const Modifier&) const = default;
    };

    inline double apply_modifier(double current, const Modifier& modifier) {
        switch (modifier.operation) {
            case ModifierOperation::Override:
                return modifier.value;

            case ModifierOperation::Increase:
                return current + modifier.value;

            case ModifierOperation::Decrease:
                return current - modifier.value;

            case ModifierOperation::Multiply:
                return current * modifier.value;
        }
    }

    // Single source: app::coyote::pulse::Channel.
    using Channel = app::coyote::pulse::Channel;

    struct ChannelModifiers {
        // nullopt means not modified.
        std::optional<Modifier> a;
        std::optional<Modifier> b;

        bool operator==(const ChannelModifiers&) const = default;
    };


    enum class StaticSource {
        Health,
        Collectible,
    };

    struct CollectibleContributionDetail {
        int id = 0;
        std::string name;
        int quality = 0;
        int count = 0;
        bool matched = false;
        std::string rule_source;
        ChannelModifiers modifiers;

        bool operator==(const CollectibleContributionDetail&) const = default;
    };

    enum class ExplanationKind {
        Static,
        Event,
    };

    struct ExplanationStep {
        std::optional<Modifier> a_modifier;
        std::optional<Modifier> b_modifier;
        double before_a = 0.0;
        double after_a = 0.0;
        double before_b = 0.0;
        double after_b = 0.0;
        std::optional<int> remaining_seconds;

        bool operator==(const ExplanationStep&) const = default;
    };

    struct RuleExplanation {
        RuleId rule_id;
        std::string name;
        ExplanationKind kind = ExplanationKind::Static;
        std::string source;
        std::vector<ExplanationStep> steps;
        std::vector<CollectibleContributionDetail> collectible_details;

        bool operator==(const RuleExplanation&) const = default;
    };

    struct StaticContribution {
        RuleId rule_id;
        std::string name;
        RuleOrder rule_order = 0;
        StaticSource source = StaticSource::Health;
        std::string source_detail;

        ChannelModifiers modifiers;
        std::vector<CollectibleContributionDetail> collectible_details;

        bool stale = false;
        std::optional<std::string> stale_reason;
    };

}

// Effect
// Order: RuleOrder -> TriggerSequence -> ActionOrder
namespace app::rule
{
    struct EffectOrder {
        RuleOrder rule = 0;
        TriggerSequence trigger = 0;
        ActionOrder action = 0;

        auto operator<=>(const EffectOrder&) const = default;
    };

    struct ActiveEffect {
        RuleId rule_id;
        std::string name;
        EffectOrder order;
        std::optional<EventContext> event_context;

        ChannelModifiers modifiers;

        std::chrono::steady_clock::time_point created_at{};
        std::optional<std::chrono::steady_clock::time_point> expires_at;
        std::string source_detail;
    };
}


#endif //ISAACCOYOTE_RULE_TYPE_H
