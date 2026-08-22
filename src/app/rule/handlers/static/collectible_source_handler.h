#ifndef ISAACCOYOTE_COLLECTIBLE_SOURCE_HANDLER_H
#define ISAACCOYOTE_COLLECTIBLE_SOURCE_HANDLER_H

#include "app/rule/handlers/rule_handler.h"

namespace app::rule
{
    class CollectibleSourceHandler final : public StaticRuleHandler {
    public:
    void evaluate(std::vector<StaticContribution>& out) override;
    };
}

#endif // ISAACCOYOTE_COLLECTIBLE_SOURCE_HANDLER_H
