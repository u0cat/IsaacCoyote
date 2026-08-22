//
// Created by TsCat on 2026/7/17.
//

#include "app/game.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "app/service/log/log_service.h"
#include "isaac_spy/isaac/game.h"
#include "isaac_spy/isaac/manager.h"

namespace app::game
{
    namespace
    {
        spdlog::logger& log_()
        {
            static auto logger = app::log::get("app.game");
            return *logger;
        }
    }

    Game::Game(const config::ConfigService& config, event::EventEngine& event_engine, coyote::CoyoteService& coyote_service)
        : config_(config), event_engine_(event_engine), coyote_service_(coyote_service) {}

    Game::~Game() {
        for (const auto& entry : event_callbacks_) event_engine_.off(entry.second);
    }

    void Game::tick() {
        const auto now = std::chrono::steady_clock::now();

        const auto snapshot = config_.snapshot();
        if (snapshot != config_snapshot_) reload_rules(snapshot);

        const bool in_game = isaac_spy::isaac::Manager::get_instance().is_in_game();
        if (in_game != was_in_game_) {
            log_().debug("in_game changed: {} -> {}", was_in_game_, in_game);
            reset_runtime();
            was_in_game_ = in_game;
        }
        if (!rules_)
            return;

        event_engine_.tick();
        rule_engine_.tick(config_snapshot_->game, now);

        for (const auto& pulse : rule_engine_.take_pulses()) {
            if (pulse.shake) {
                int frame_count = pulse.shake_duration.count() / 60;
                isaac_spy::isaac::Game::get_instance().shake_screen(frame_count);
            }
            coyote_service_.push_pulse(pulse.pulse_a, pulse.pulse_b, pulse.duration);
        }

        const auto output = rule_engine_.output();
        coyote_service_.tick(
            now,
            static_cast<int>(std::lround(output.a)),
            static_cast<int>(std::lround(output.b))
        );
    }

    rule::StateHub& Game::state_hub() noexcept {
        return rule_engine_.state_hub();
    }

    coyote::CoyoteService::MonitorSnapshot Game::pulse_monitor_snapshot() const {
        return coyote_service_.monitor_snapshot();
    }

    void Game::reload_rules(config::ConfigService::Snapshot snapshot) {
        coyote_service_.reload_pulses();
        const auto compiled = rule_engine_.compile(*snapshot);
        if (!compiled.success()) {
            return;
        }
        config_snapshot_ = std::move(snapshot);
        rules_ = compiled.rules;
        update_event_subscriptions();
        rule_engine_.configure(*rules_);
        log_().debug("rules reloaded: {} static, {} event rules",
                     rules_->static_rules.size(), rules_->event_rules.size());
        if (config_snapshot_->game.constant_mode.enabled) {
            coyote_service_.set_loop(
                config_snapshot_->game.constant_mode.pulse_a,
                config_snapshot_->game.constant_mode.pulse_b
            );
        }
        else {
            coyote_service_.set_loop({}, {});
        }
        reset_runtime();
    }

    void Game::update_event_subscriptions() {
        std::set<event::EventType> required_types;
        for (const auto& rule : rules_->event_rules)
            required_types.insert(event::event_kind_of(rule.event));

        std::erase_if(event_callbacks_, [this, &required_types](const auto& entry) {
            if (required_types.contains(entry.first))
                return false;
            event_engine_.off(entry.second);
            return true;
        });

        for (const auto type : required_types) {
            if (event_callbacks_.contains(type))
                continue;

            auto result = event_engine_.on(type, [this](const event::EventVariant& event) {
                const auto now = std::chrono::steady_clock::now();
                rule_engine_.on_event(event, now);
            });
            if (result) {
                event_callbacks_.emplace(type, *result);
            }
            else {
                log_().error("failed to enable event source {}: {}",
                             static_cast<int>(type), result.error().message);
            }
        }
    }

    void Game::reset_runtime() {
        rule_engine_.reset();
        event_engine_.reset();
    }
}
