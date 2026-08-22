//
// Created by TsCat on 2026/7/25.
//

#include "app/event/sources/reroll.h"

#include "app/event/event_engine.h"
#include "app/service/log/log_service.h"
#include "isaac_spy/isaac/game.h"
#include "isaac_spy/isaac/manager.h"

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.event");
        return *logger;
    }
}

using namespace app::event::sources;

RerollSource::RerollSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager)
    : IEventSource(engine),
      start_new_game_hook_(hook_manager.get_hook<isaac_spy::hook::StartNewGameHook>()),
      restart_game_hook_(hook_manager.get_hook<isaac_spy::hook::RestartGameHook>()) {}

std::expected<void, EventSourceError> RerollSource::enable() {
    auto start = start_new_game_hook_.subscribe([this](const isaac_spy::hook::StartNewGameContext&)
    {
        new_game_flag_ = true;
    });
    if (!start) return std::unexpected(EventSourceError{start.error().message});

    auto restart = restart_game_hook_.subscribe([this](const isaac_spy::hook::RestartGameContext& ctx)
    {
        if (ctx.from_console) return;
        restart_flag_ = true;
    });
    if (!restart) {
        start_new_game_hook_.unsubscribe(*start);
        return std::unexpected(EventSourceError{restart.error().message});
    }

    start_subscription_ = *start;
    restart_subscription_ = *restart;
    return {};
}

void RerollSource::disable() {
    start_new_game_hook_.unsubscribe(start_subscription_);
    restart_game_hook_.unsubscribe(restart_subscription_);
    start_subscription_ = 0;
    restart_subscription_ = 0;
    new_game_flag_ = false;
    restart_flag_ = false;
    can_continue_ = false;
}

void RerollSource::tick() {
    if (new_game_flag_) {
        new_game_flag_ = false;
        if (can_continue_) {
            log_().info("[Reroll] New Game");
            engine_.post(RerollGameEvent{});
        }
    }

    if (restart_flag_) {
        restart_flag_ = false;
        if (!isaac_spy::isaac::Game::get_instance().is_game_over()) {
            log_().info("[Reroll] R Key");
            engine_.post(RerollGameEvent{});
        }
    }

    can_continue_ = isaac_spy::isaac::Manager::get_instance().can_continue();
}

void RerollSource::reset() {
    // A restart can cross an out-of-game frame; keep events until tick consumes them.
}
