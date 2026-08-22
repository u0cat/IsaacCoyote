// Created by TsCat on 2026/7/23.

#include "app/rule/rule_handler_manager.h"

#include "app/rule/rule_utils.h"

namespace app::rule
{
    void RuleHandlerManager::configure(const CompiledRules& rules) {
        std::unordered_map<event::EventType, std::vector<CompiledEventRule>> event_rules;
        std::unordered_map<StaticSource, std::vector<CompiledStaticRule>> static_rules;

        for (const auto& rule : rules.event_rules)
            event_rules[event::event_kind_of(rule.event)].push_back(rule);
        for (const auto& rule : rules.static_rules)
            static_rules[source_kind_of(rule.source)].push_back(rule);

        for (std::size_t i = 0; i < event_handlers_.size(); ++i) {
            auto& handler = event_handlers_[i];
            handler->configure(std::move(event_rules[static_cast<event::EventType>(i)]));
            handler->set_effects_target(all_effects_);
        }
        for (std::size_t i = 0; i < static_handlers_.size(); ++i) {
            static_handlers_[i]->configure(std::move(static_rules[static_cast<StaticSource>(i)]));
        }
        all_effects_.clear();
        all_contributions_.clear();
    }

    void RuleHandlerManager::dispatch_event(
        const event::EventVariant& event,
        const std::chrono::steady_clock::time_point now
    ) {
        const std::size_t index = static_cast<std::size_t>(event::event_type_of(event));
        if (index < event_handlers_.size() && event_handlers_[index])
            event_handlers_[index]->on_event(event, now);
    }

    void RuleHandlerManager::tick(const std::chrono::steady_clock::time_point now) {
        all_contributions_.clear();
        for (auto& handler : static_handlers_)
            handler->evaluate(all_contributions_);

        std::erase_if(all_effects_, [now](const ActiveEffect& effect)
        {
            return effect.expires_at && *effect.expires_at <= now;
        });
    }

    std::span<const StaticContribution> RuleHandlerManager::all_contributions() const {
        return all_contributions_;
    }

    std::span<const ActiveEffect> RuleHandlerManager::all_effects() const {
        return all_effects_;
    }

    std::vector<CompiledPulseAction> RuleHandlerManager::take_all_pulses() {
        std::vector<CompiledPulseAction> result;
        for (auto& handler : event_handlers_) {
            auto pulses = handler->take_pulses();
            result.insert(result.end(),
                          std::make_move_iterator(pulses.begin()),
                          std::make_move_iterator(pulses.end()));
        }
        return result;
    }

    void RuleHandlerManager::reset() {
        for (auto& handler : event_handlers_)
            handler->reset();
        for (auto& handler : static_handlers_)
            handler->reset();
        all_contributions_.clear();
        all_effects_.clear();
    }
}
