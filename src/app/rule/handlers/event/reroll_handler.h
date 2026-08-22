#ifndef ISAACCOYOTE_REROLL_GAME_HANDLER_H
#define ISAACCOYOTE_REROLL_GAME_HANDLER_H

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class RerollGameHandler final : public EventRuleHandler {
    public:
        void on_event(const event::EventVariant& event, std::chrono::steady_clock::time_point now) override;
        void reset() override;
    };
}

#endif // ISAACCOYOTE_REROLL_GAME_HANDLER_H
