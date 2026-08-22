#ifndef ISAACCOYOTE_RULE_HANDLER_H
#define ISAACCOYOTE_RULE_HANDLER_H

#include <cassert>
#include <chrono>
#include <unordered_map>
#include <utility>
#include <vector>

#include "app/rule/rule_utils.h"

namespace app::rule
{
    class IRuleHandler {
    public:
        virtual ~IRuleHandler() = default;
        virtual void reset() = 0;
    };

    class IEventRuleHandler : public IRuleHandler {
    public:
        virtual void configure(std::vector<CompiledEventRule> rules) = 0;
        virtual void on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now) = 0;
        virtual std::vector<CompiledPulseAction> take_pulses() = 0;
        virtual void set_effects_target(std::vector<ActiveEffect>& target) = 0;
    };

    class IStaticRuleHandler : public IRuleHandler {
    public:
        virtual void configure(std::vector<CompiledStaticRule> rules) = 0;
        virtual void evaluate(std::vector<StaticContribution>& out) = 0;
    };

    class EventRuleHandler : public IEventRuleHandler {
    public:
        void configure(std::vector<CompiledEventRule> rules) final {
            reset();
            rules_ = std::move(rules);
        }

        void reset() {
            pending_pulses_.clear();
            trigger_sequence_ = 0;
        }

        std::vector<CompiledPulseAction> take_pulses() final {
            return std::exchange(pending_pulses_, {});
        }

        void set_effects_target(std::vector<ActiveEffect>& target) final {
            effects_target_ = &target;
        }

    protected:
        void process(
            const std::optional<EventContext>& context,
            const event::EventVariant& event,
            const std::chrono::steady_clock::time_point now
        ) {
            assert(effects_target_ != nullptr);
            process_event_rules(
                rules_, context, now, trigger_sequence_,
                [this, &event](const CompiledEventRule& rule) { return extra_match(rule, event); },
                *effects_target_, pending_pulses_
            );
        }

        virtual bool extra_match(const CompiledEventRule& rule, const event::EventVariant& event) const {
            (void)rule;
            (void)event;
            return true;
        }

        void process_global(
            const event::EventVariant& event,
            const std::chrono::steady_clock::time_point now
        ) {
            process_event_rules(
                rules_, std::nullopt, now, trigger_sequence_,
                [this, &event](const CompiledEventRule& rule) { return extra_match(rule, event); },
                *effects_target_, pending_pulses_,
                true
            );
        }

        std::vector<CompiledEventRule> rules_;
        std::vector<CompiledPulseAction> pending_pulses_;
        TriggerSequence trigger_sequence_ = 0;
        std::vector<ActiveEffect>* effects_target_ = nullptr;
    };

    class StaticRuleHandler : public IStaticRuleHandler {
    public:
        void configure(std::vector<CompiledStaticRule> rules) final {
            reset();
            rules_ = std::move(rules);
        }

        void reset() {
            cache_.clear();
        }

    protected:
        std::vector<CompiledStaticRule> rules_;
        std::unordered_map<RuleId, StaticContribution> cache_;
    };
}

#endif // ISAACCOYOTE_RULE_HANDLER_H