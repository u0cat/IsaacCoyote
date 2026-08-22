#ifndef ISAACCOYOTE_COMPILED_RULES_H
#define ISAACCOYOTE_COMPILED_RULES_H

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "app/event/catalog.h"
#include "rule_type.h"

namespace app::rule
{
    struct CompiledPlayerFilter {
        PlayerScope scope = PlayerScope::Any;
        std::unordered_set<PlayerId> player_ids;
    };

    struct CompiledStrengthAction {
        ChannelModifiers modifiers;
        std::optional<std::chrono::milliseconds> duration;
    };

    struct CompiledPulseAction {
        std::string pulse_a;
        std::string pulse_b;
        std::chrono::milliseconds duration{0};

        bool shake = false;
        std::chrono::milliseconds shake_duration{0};
    };

    using CompiledEventAction = std::variant<CompiledStrengthAction, CompiledPulseAction>;

    // Per-event compiled shapes are defined by the descriptors in app/event/descriptors/
    using CompiledEventConfig = app::event::CompiledEventConfig;

    struct CompiledEventRule {
        RuleId rule_id;
        std::string name;
        RuleOrder order = 0;
        CompiledPlayerFilter players;
        CompiledEventConfig event;
        std::vector<CompiledEventAction> actions;
    };

    struct CompiledHealthSource {
        ChannelModifiers per_red_heart;
    };

    struct CompiledCollectibleSource {
        std::unordered_map<int, ChannelModifiers> modifiers_by_quality;
        std::unordered_map<int, ChannelModifiers> override_rule;
    };

    using CompiledStaticSource = std::variant<CompiledHealthSource, CompiledCollectibleSource>;

    struct CompiledStaticRule {
        RuleId rule_id;
        std::string name;
        RuleOrder order = 0;
        CompiledPlayerFilter players;
        CompiledStaticSource source;
    };

    struct CompiledRules {
        std::vector<CompiledStaticRule> static_rules;
        std::vector<CompiledEventRule> event_rules;
    };

    struct CompileResult {
        std::shared_ptr<const CompiledRules> rules;

        [[nodiscard]] bool success() const { return rules != nullptr; }
    };

}

#endif // ISAACCOYOTE_COMPILED_RULES_H