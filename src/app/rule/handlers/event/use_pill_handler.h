#ifndef ISAACCOYOTE_USE_PILL_HANDLER_H
#define ISAACCOYOTE_USE_PILL_HANDLER_H

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class UsePillHandler final : public EventRuleHandler {
    public:
        void on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now) override;

    protected:
        bool extra_match(const CompiledEventRule& rule, const event::EventVariant& event) const override;
    };
}

#endif // ISAACCOYOTE_USE_PILL_HANDLER_H
