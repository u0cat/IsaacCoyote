#ifndef ISAACCOYOTE_RULE_UTILS_H
#define ISAACCOYOTE_RULE_UTILS_H

#include <chrono>
#include <functional>
#include <optional>
#include <span>
#include <vector>

#include "app/event/catalog.h"
#include "app/rule/types/compiled_rules.h"
#include "app/service/config/config_struct.h"
#include "isaac_spy/isaac/entity_ref.h"

namespace app::rule
{
    ModifierOperation compile_operation(config::StrengthModifier operation);
    std::optional<Modifier> compile_modifier(const std::optional<config::ModifierConfig>& modifier);
    ChannelModifiers compile_modifiers(const config::ChannelModifiersConfig& modifiers);
    std::optional<CompiledPlayerFilter> compile_player_filter(const config::PlayerFilterConfig& filter);

    bool player_matches(const CompiledPlayerFilter& filter, const std::optional<EventContext>& context);
    bool entity_blacklisted(const CompiledEntityFilter& filter, const isaac_spy::isaac::Entity& entity);
    bool entity_whitelisted(const CompiledEntityFilter& filter, const isaac_spy::isaac::Entity& entity);
    bool item_blacklisted(const CompiledItemFilter& filter, int item_id);
    bool item_whitelisted(const CompiledItemFilter& filter, int item_id);
    ChannelModifiers repeated_modifiers(const ChannelModifiers& modifiers, double count);
    ChannelModifiers merge_modifiers(const std::vector<ChannelModifiers>& modifiers);
    StaticSource source_kind_of(const CompiledStaticSource& source);

    template <typename ConditionMatcher>
    void process_event_rules(
        const std::span<const CompiledEventRule> rules,
        const std::optional<EventContext>& context,
        const std::chrono::steady_clock::time_point now,
        TriggerSequence& trigger_sequence,
        ConditionMatcher&& extra_match,
        std::vector<ActiveEffect>& effects,
        std::vector<CompiledPulseAction>& pulses,
        bool ignore_player_filter = false
    ) {
        const auto trigger = trigger_sequence++;

        for (const auto& rule : rules) {
            if ((!ignore_player_filter && !player_matches(rule.players, context)) || !std::invoke(extra_match, rule))
                continue;

            for (std::size_t action_index = 0; action_index < rule.actions.size(); ++action_index) {
                const auto& action = rule.actions[action_index];
                if (const auto* strength = std::get_if<CompiledStrengthAction>(&action)) {
                    ActiveEffect effect;
                    effect.rule_id = rule.rule_id;
                    effect.name = rule.name;
                    effect.order = {rule.order, trigger, static_cast<ActionOrder>(action_index)};
                    effect.event_context = context;
                    effect.modifiers = strength->modifiers;
                    effect.created_at = now;
                    if (strength->duration)
                        effect.expires_at = now + *strength->duration;
                    effect.source_detail = "动作 " + std::to_string(action_index + 1);
                    effects.push_back(std::move(effect));
                }

                if (const auto* pulse = std::get_if<CompiledPulseAction>(&action))
                    pulses.push_back(*pulse);
            }
        }
    }
}

#endif // ISAACCOYOTE_RULE_UTILS_H
