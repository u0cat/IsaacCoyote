#ifndef ISAACCOYOTE_HEALTH_SOURCE_HANDLER_H
#define ISAACCOYOTE_HEALTH_SOURCE_HANDLER_H

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class HealthSourceHandler final : public StaticRuleHandler {
    public:
    void evaluate(std::vector<StaticContribution>& out) override;
    };
}

#endif // ISAACCOYOTE_HEALTH_SOURCE_HANDLER_H
