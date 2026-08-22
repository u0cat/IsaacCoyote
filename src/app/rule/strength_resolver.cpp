// Created by TsCat on 2026/7/17.

#include "app/rule/strength_resolver.h"

#include <algorithm>
#include <map>
#include <span>
#include <vector>

namespace app::rule
{
    namespace
    {
        void apply(ChannelModifiers modifiers, StrengthPair& value)
        {
            if (modifiers.a) value.a = apply_modifier(value.a, *modifiers.a);
            if (modifiers.b) value.b = apply_modifier(value.b, *modifiers.b);
        }

        void update_channel(OutputSmoother::ChannelState& state,
                            const double target,
                            const config::ClimbConfig& climb,
                            const config::DecayConfig& decay,
                            const std::chrono::steady_clock::time_point now)
        {
            state.target = target;
            if (target == state.output) {
                state.output = target;
                state.direction = OutputSmoother::Direction::None;
                return;
            }

            const auto direction = target > state.output
                                       ? OutputSmoother::Direction::Climbing
                                       : OutputSmoother::Direction::Decaying;
            const bool enabled = direction == OutputSmoother::Direction::Climbing
                                     ? climb.enabled
                                     : decay.enabled;
            const auto interval = direction == OutputSmoother::Direction::Climbing
                                      ? climb.interval
                                      : decay.interval;
            const double amount = direction == OutputSmoother::Direction::Climbing
                                      ? climb.amount
                                      : decay.amount;

            if (!enabled || interval.count() <= 0 || amount <= 0.0) {
                state.output = target;
                state.direction = OutputSmoother::Direction::None;
                return;
            }

            if (state.direction != direction) {
                state.direction = direction;
                state.last_step = now;
            }

            const auto steps = (now - state.last_step) / interval;
            if (steps <= 0) return;

            const double change = static_cast<double>(steps) * amount;
            if (direction == OutputSmoother::Direction::Climbing)
                state.output = std::min(state.target, state.output + change);
            else
                state.output = std::max(state.target, state.output - change);

            if (state.output == state.target)
                state.direction = OutputSmoother::Direction::None;
            state.last_step += interval * steps;
        }

        // Stage 1: normalize into one ordered step stream; static contributions (by rule_order)
        // come before effects (by EffectOrder).
        struct ResolveStep
        {
            const StaticContribution* static_contribution = nullptr;
            const ActiveEffect* effect = nullptr;
            StrengthPair before{};
            StrengthPair after{};
        };

        std::vector<ResolveStep> collect_steps(
            const std::span<const StaticContribution> static_contributions,
            const std::span<const ActiveEffect> effects)
        {
            std::vector<ResolveStep> steps;
            steps.reserve(static_contributions.size() + effects.size());

            for (const auto& contribution : static_contributions)
                steps.push_back({.static_contribution = &contribution});
            for (const auto& effect : effects) steps.push_back({.effect = &effect});

            const auto static_end = steps.begin() + static_cast<std::ptrdiff_t>(static_contributions.size());
            std::ranges::stable_sort(steps.begin(), static_end, {},
                                     [](const ResolveStep& step) { return step.static_contribution->rule_order; });
            std::ranges::stable_sort(static_end, steps.end(), {},
                                     [](const ResolveStep& step) { return step.effect->order; });
            return steps;
        }

        // Stage 2: pure math fold; each step records the target before and after its modifiers.
        void apply_steps(const std::span<ResolveStep> steps, StrengthPair& target)
        {
            for (auto& step : steps) {
                step.before = target;
                const auto& modifiers = step.static_contribution
                                            ? step.static_contribution->modifiers
                                            : step.effect->modifiers;
                apply(modifiers, target);
                step.after = target;
            }
        }

        // Stage 3: summaries — one per static contribution, effects grouped by (rule_order, rule_id).
        void build_summaries(
            const std::span<const ResolveStep> steps,
            std::vector<RuleContributionSummary>& summaries)
        {
            std::map<std::pair<RuleOrder, RuleId>, std::size_t> counts;
            for (const auto& step : steps) {
                if (step.static_contribution) {
                    summaries.push_back({
                        step.static_contribution->rule_id,
                        step.static_contribution->rule_order,
                        1,
                        step.static_contribution->stale,
                        step.static_contribution->stale_reason,
                    });
                }
                else {
                    ++counts[{step.effect->order.rule, step.effect->rule_id}];
                }
            }
            for (const auto& [key, count] : counts)
                summaries.push_back({key.second, key.first, count, false, std::nullopt});
        }

        // Stage 4: explanations from before/after values; the countdown (remaining_seconds)
        // is computed here, outside the strength math.
        void build_explanations(
            const std::span<const ResolveStep> steps,
            const std::chrono::steady_clock::time_point now,
            std::vector<RuleExplanation>& explanations)
        {
            for (const auto& step : steps) {
                const bool is_static = step.static_contribution != nullptr;
                const auto& rule_id = is_static ? step.static_contribution->rule_id : step.effect->rule_id;
                const auto& name = is_static ? step.static_contribution->name : step.effect->name;
                const auto& source_detail = is_static ? step.static_contribution->source_detail
                                                      : step.effect->source_detail;
                const auto& modifiers = is_static ? step.static_contribution->modifiers
                                                  : step.effect->modifiers;

                std::optional<int> remaining;
                if (!is_static && step.effect->expires_at) {
                    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(*step.effect->expires_at - now).count();
                    remaining = static_cast<int>(std::max<std::int64_t>(0, seconds));
                }

                RuleExplanation explanation{
                    .rule_id = rule_id,
                    .name = name.empty() ? rule_id : name,
                    .kind = is_static ? ExplanationKind::Static : ExplanationKind::Event,
                    .source = source_detail,
                };
                explanation.steps.push_back({
                    .a_modifier = modifiers.a,
                    .b_modifier = modifiers.b,
                    .before_a = step.before.a,
                    .after_a = step.after.a,
                    .before_b = step.before.b,
                    .after_b = step.after.b,
                    .remaining_seconds = remaining,
                });
                if (is_static)
                    explanation.collectible_details = step.static_contribution->collectible_details;
                explanations.push_back(std::move(explanation));
            }
        }
    }

    ResolveResult StrengthResolver::resolve(
        const config::StrengthConfig& strength,
        const std::span<const app::rule::StaticContribution> static_contributions,
        const std::span<const app::rule::ActiveEffect> effects,
        const std::chrono::steady_clock::time_point now) const
    {
        ResolveResult result{
            .target = {strength.base_a, strength.base_b},
            .unclamped_target = {strength.base_a, strength.base_b},
        };
        result.explanations.reserve(static_contributions.size() + effects.size());

        auto steps = collect_steps(static_contributions, effects);
        apply_steps(steps, result.target);
        build_summaries(steps, result.summaries);
        build_explanations(steps, now, result.explanations);

        result.unclamped_target = result.target;
        result.target.a = std::clamp(result.target.a, 0.0, strength.limit_a);
        result.target.b = std::clamp(result.target.b, 0.0, strength.limit_b);
        return result;
    }

    StrengthPair OutputSmoother::update(
        const StrengthPair target,
        const config::ClimbConfig& climb,
        const config::DecayConfig& decay,
        const std::chrono::steady_clock::time_point now)
    {
        if (!initialized_) {
            a_ = {target.a, target.a, Direction::None, now};
            b_ = {target.b, target.b, Direction::None, now};
            initialized_ = true;
            return target;
        }

        update_channel(a_, target.a, climb, decay, now);
        update_channel(b_, target.b, climb, decay, now);
        return {a_.output, b_.output};
    }

    void OutputSmoother::reset() noexcept
    {
        a_ = {};
        b_ = {};
        initialized_ = false;
    }
}