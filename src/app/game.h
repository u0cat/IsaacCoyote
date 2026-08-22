//
// Created by TsCat on 2026/7/17.
//

#ifndef ISAACCOYOTE_GAME_H
#define ISAACCOYOTE_GAME_H

#include <memory>
#include <unordered_map>

#include "app/event/event_engine.h"
#include "app/rule/rule_engine.h"
#include "app/service/config/config_service.h"
#include "service/coyote/coyote_service.h"

namespace app::game
{
    class Game {
    public:
        Game(const config::ConfigService& config, event::EventEngine& event_engine, coyote::CoyoteService& coyote_service);
        ~Game();

        void tick();
        [[nodiscard]] rule::StateHub& state_hub() noexcept;
        [[nodiscard]] coyote::CoyoteService::MonitorSnapshot pulse_monitor_snapshot() const;

    private:
        void reload_rules(config::ConfigService::Snapshot snapshot);
        void update_event_subscriptions();
        void reset_runtime();

        config::ConfigService::Snapshot config_snapshot_;
        std::shared_ptr<const rule::CompiledRules> rules_;

        rule::RuleEngine rule_engine_;

        event::EventEngine& event_engine_;
        const config::ConfigService& config_;
        coyote::CoyoteService& coyote_service_;

        std::unordered_map<event::EventType, event::EventEngine::CallbackId> event_callbacks_;
        bool was_in_game_ = false;
    };
}
#endif //ISAACCOYOTE_GAME_H
