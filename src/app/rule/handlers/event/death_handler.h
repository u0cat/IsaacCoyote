#ifndef ISAACCOYOTE_DEATH_HANDLER_H
#define ISAACCOYOTE_DEATH_HANDLER_H

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class DeathHandler final : public EventRuleHandler {
    public:
        void on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now) override;
    };
}

#endif // ISAACCOYOTE_DEATH_HANDLER_H
