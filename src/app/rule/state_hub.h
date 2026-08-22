//
// Created by TsCat on 2026/7/17.
//

#ifndef ISAACCOYOTE_STATE_HUB_H
#define ISAACCOYOTE_STATE_HUB_H

#include <chrono>
#include <memory>
#include <vector>

#include "app/rule/strength_resolver.h"

namespace app::rule
{
    struct StateSnapshot {
        std::chrono::steady_clock::time_point created_at{};
        StrengthPair output;
        StrengthPair base;
        StrengthPair target;
        StrengthPair unclamped_target;
        StrengthPair limits;
        bool stale = false;
        bool reset_pending = false;
        std::vector<RuleContributionSummary> contributions;
        std::vector<RuleExplanation> explanations;

        bool operator==(const StateSnapshot& other) const {
            return output == other.output
                && base == other.base
                && target == other.target
                && unclamped_target == other.unclamped_target
                && limits == other.limits
                && stale == other.stale
                && reset_pending == other.reset_pending
                && contributions == other.contributions
                && explanations == other.explanations;
        }
    };

    class StateHub {
    public:
        StateHub();

        void publish(StateSnapshot snapshot);
        void mark_reset_pending();
        std::shared_ptr<const StateSnapshot> snapshot() const;

    private:
        std::shared_ptr<const StateSnapshot> snapshot_;
    };
}

#endif //ISAACCOYOTE_STATE_HUB_H
