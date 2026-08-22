#ifndef ISAACCOYOTE_RULE_HANDLER_MANAGER_H
#define ISAACCOYOTE_RULE_HANDLER_MANAGER_H

#include <array>
#include <memory>
#include <span>
#include <type_traits>

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class RuleHandlerManager {
    public:
        template<typename T, typename... Args>
        void register_event_handler(const event::EventType type, Args&&... args)
        {
            event_handlers_.at(static_cast<std::size_t>(type)) =
                std::make_unique<T>(std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void register_static_handler(const StaticSource type, Args&&... args)
        {
            static_handlers_.at(static_cast<std::size_t>(type)) =
                std::make_unique<T>(std::forward<Args>(args)...);
        }

        void configure(const CompiledRules& rules);
        void dispatch_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now);
        void tick(std::chrono::steady_clock::time_point now);

        // Valid until the next tick / configure / reset — consume within one tick.
        [[nodiscard]] std::span<const StaticContribution> all_contributions() const;
        [[nodiscard]] std::span<const ActiveEffect> all_effects() const;
        std::vector<CompiledPulseAction> take_all_pulses();
        void reset();

    private:
        // EventType::None is the sentinel after the real event kinds.
        static constexpr std::size_t kEventHandlerCount =
            static_cast<std::size_t>(event::EventType::None);
        static constexpr std::size_t kStaticHandlerCount =
            static_cast<std::size_t>(StaticSource::Collectible) + 1;

        std::array<std::unique_ptr<IEventRuleHandler>, kEventHandlerCount> event_handlers_;
        std::array<std::unique_ptr<IStaticRuleHandler>, kStaticHandlerCount> static_handlers_;
        std::vector<StaticContribution> all_contributions_;
        std::vector<ActiveEffect> all_effects_;
    };
}

#endif // ISAACCOYOTE_RULE_HANDLER_MANAGER_H
