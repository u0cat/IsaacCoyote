#include "app/rule/rule_engine.h"

#include <type_traits>

#include "app/rule/handlers/event/death_handler.h"
#include "app/rule/handlers/event/game_over_handler.h"
#include "app/rule/handlers/event/hurt_handler.h"
#include "app/rule/handlers/event/reroll_handler.h"
#include "app/rule/handlers/event/use_active_item_handler.h"
#include "app/rule/handlers/event/use_card_handler.h"
#include "app/rule/handlers/event/use_pill_handler.h"
#include "app/rule/handlers/static/collectible_source_handler.h"
#include "app/rule/handlers/static/health_source_handler.h"
namespace app::rule
{
    RuleEngine::RuleEngine() {
        event::for_each_event_type([&]<event::EventType T>()
        {
            handler_manager_.register_event_handler<typename event::EventDescriptor<T>::Handler>(T);
        });

        handler_manager_.register_static_handler<HealthSourceHandler>(StaticSource::Health);
        handler_manager_.register_static_handler<CollectibleSourceHandler>(StaticSource::Collectible);
    }

    CompileResult RuleEngine::compile(const config::AppConfig& config) const {
        auto compiled = std::make_shared<CompiledRules>();
        std::unordered_set<RuleId> rule_ids;

        const auto validate_id = [&](const RuleId& id)
        {
            return !id.empty() && rule_ids.insert(id).second;
        };

        // Static
        for (std::size_t index = 0; index < config.game.static_rules.size(); ++index) {
            const auto& source_rule = config.game.static_rules[index];
            if (!source_rule.enabled || !validate_id(source_rule.rule_id))
                continue;

            auto players = compile_player_filter(source_rule.players);
            if (!players) continue;

            CompiledStaticSource source = std::visit([]<typename T>(const T& source_config) -> CompiledStaticSource
            {
                if constexpr (std::is_same_v<T, config::HealthSourceConfig>) {
                    return CompiledHealthSource{compile_modifiers(source_config.per_red_heart)};
                }
                else if constexpr (std::is_same_v<T, config::CollectibleSourceConfig>) {
                    CompiledCollectibleSource result;
                    for (const auto& [quality, modifiers] : source_config.modifiers_by_quality)
                        result.modifiers_by_quality.emplace(quality, compile_modifiers(modifiers));
                    for (const auto& [item_id, modifiers] : source_config.override_rule)
                        result.override_rule.emplace(item_id, compile_modifiers(modifiers));
                    return result;
                }
            }, source_rule.source);

            compiled->static_rules.push_back({
                source_rule.rule_id,
                source_rule.name.empty() ? source_rule.rule_id : source_rule.name,
                static_cast<RuleOrder>(index),
                std::move(*players),
                std::move(source),
            });
        }

        // Event
        for (std::size_t index = 0; index < config.game.events.size(); ++index) {
            const auto& source_rule = config.game.events[index];
            if (!source_rule.enabled || !validate_id(source_rule.rule_id))
                continue;

            const event::EventType event_kind = event::event_kind_of(source_rule.event);
            if (event_kind == event::EventType::None) {
                continue;
            }

            std::optional<CompiledPlayerFilter> players = std::visit(
                [&](const auto& event_config) -> std::optional<CompiledPlayerFilter>
                {
                    using T = std::decay_t<decltype(event_config)>;
                    if constexpr (std::is_same_v<T, event::NoneEventConfig>) return std::nullopt;
                    else return compile_player_filter(event_config.players);
                },
                source_rule.event
            );
            if (!players)
                continue;

            CompiledEventRule rule{
                source_rule.rule_id,
                source_rule.name.empty() ? source_rule.rule_id : source_rule.name,
                static_cast<RuleOrder>(index),
                std::move(*players),
                event::compile_event_config(source_rule.event),
                {},
            };

            for (const auto& action : source_rule.actions) {
                if (const auto* strength = std::get_if<config::StrengthEffectConfig>(&action)) {
                    if (strength->temporary && (!strength->duration || strength->duration->count() <= 0)) {
                        continue;
                    }
                    rule.actions.emplace_back(CompiledStrengthAction{
                        compile_modifiers(strength->modifiers),
                        strength->temporary ? strength->duration : std::nullopt,
                    });
                    continue;
                }

                if (const auto& pulse = std::get_if<config::PulseEffectConfig>(&action)) {
                    if (pulse->duration.count() < 0) {
                        continue;
                    }

                    if (pulse->shake_duration.count() < 0) {
                        continue;
                    }

                    CompiledPulseAction compiled_action;
                    compiled_action.duration = pulse->duration;

                    compiled_action.pulse_a = pulse->pulse_a;
                    compiled_action.pulse_b = pulse->pulse_b;

                    compiled_action.shake = pulse->shake;
                    compiled_action.shake_duration = pulse->shake_duration;
                    rule.actions.emplace_back(std::move(compiled_action));
                }
            }

            compiled->event_rules.push_back(std::move(rule));
        }

        return {std::move(compiled)};
    }

    void RuleEngine::configure(const CompiledRules& rules) {
        handler_manager_.configure(rules);
    }

    void RuleEngine::tick(
        const config::GameConfig& game, std::chrono::steady_clock::time_point now
    ) {
        handler_manager_.tick(now);

        auto resolved = resolver_.resolve(
            game.strength,
            handler_manager_.all_contributions(),
            handler_manager_.all_effects(),
            now
        );

        auto output = smoother_.update(
            resolved.target, game.climb, game.decay, now
        );

        output_ = output;

        StateSnapshot state{
            .created_at = now,
            .output = output,
            .base = {game.strength.base_a, game.strength.base_b},
            .target = resolved.target,
            .unclamped_target = resolved.unclamped_target,
            .limits = {game.strength.limit_a, game.strength.limit_b},
            .stale = std::ranges::any_of(handler_manager_.all_contributions(), &StaticContribution::stale),
            .contributions = std::move(resolved.summaries),
            .explanations = std::move(resolved.explanations),
        };

        state_hub_.publish(std::move(state));
    }

    void RuleEngine::on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now) {
        handler_manager_.dispatch_event(event, now);
    }

    StrengthPair RuleEngine::output() const {
        return output_;
    }

    std::vector<CompiledPulseAction> RuleEngine::take_pulses() {
        return handler_manager_.take_all_pulses();
    }

    StateHub& RuleEngine::state_hub() noexcept {
        return state_hub_;
    }

    void RuleEngine::reset() {
        handler_manager_.reset();
        smoother_.reset();
        output_ = {};
        state_hub_.mark_reset_pending();
    }
}
