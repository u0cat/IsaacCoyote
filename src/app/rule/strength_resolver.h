#ifndef ISAACCOYOTE_STRENGTH_RESOLVER_H
#define ISAACCOYOTE_STRENGTH_RESOLVER_H

#include <chrono>
#include <span>
#include <vector>

#include "app/service/config/config_struct.h"
#include "types/rule_type.h"

namespace app::rule
{
    struct StrengthPair {
        double a = 0.0;
        double b = 0.0;

        bool operator==(const StrengthPair&) const = default;
    };

    struct RuleContributionSummary {
        RuleId rule_id;
        RuleOrder rule_order = 0;
        std::size_t instance_count = 0;
        bool stale = false;
        std::optional<std::string> stale_reason;

        bool operator==(const RuleContributionSummary&) const = default;
    };

    struct ResolveResult {
        StrengthPair target;
        StrengthPair unclamped_target;
        std::vector<RuleContributionSummary> summaries;
        std::vector<RuleExplanation> explanations;
    };

    class StrengthResolver {
    public:
        [[nodiscard]] ResolveResult resolve(
            const config::StrengthConfig& strength,
            std::span<const StaticContribution> static_contributions,
            std::span<const ActiveEffect> effects,
            std::chrono::steady_clock::time_point now) const;
    };

    class OutputSmoother {
    public:
        enum class Direction {
            None,
            Climbing,
            Decaying,
        };

        struct ChannelState {
            double output = 0.0;
            double target = 0.0;
            Direction direction = Direction::None;
            std::chrono::steady_clock::time_point last_step{};
        };

        [[nodiscard]] StrengthPair update(
            StrengthPair target,
            const config::ClimbConfig& climb,
            const config::DecayConfig& decay,
            std::chrono::steady_clock::time_point now);
        void reset() noexcept;

    private:
        ChannelState a_;
        ChannelState b_;
        bool initialized_ = false;
    };
}

#endif
