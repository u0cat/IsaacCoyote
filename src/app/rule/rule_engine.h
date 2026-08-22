#ifndef ISAACCOYOTE_RULE_ENGINE_H
#define ISAACCOYOTE_RULE_ENGINE_H

#include <memory>
#include <vector>

#include "app/rule/rule_handler_manager.h"
#include "app/rule/state_hub.h"
#include "app/rule/strength_resolver.h"
#include "app/service/config/config_struct.h"
#include "types/compiled_rules.h"

namespace app::rule
{
    class RuleEngine {
    public:
        RuleEngine();

        [[nodiscard]] CompileResult compile(const config::AppConfig& config) const;

        void configure(const CompiledRules& rules);

        void tick(const config::GameConfig& game,
                  std::chrono::steady_clock::time_point now);

        void on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now);

        [[nodiscard]] StrengthPair output() const;
        std::vector<CompiledPulseAction> take_pulses();
        StateHub& state_hub() noexcept;
        void reset();

    private:
        RuleHandlerManager handler_manager_;
        StrengthResolver resolver_;
        OutputSmoother smoother_;
        StateHub state_hub_;
        StrengthPair output_{};
    };
}

#endif // ISAACCOYOTE_RULE_ENGINE_H
