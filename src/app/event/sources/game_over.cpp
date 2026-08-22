//
// Created by TsCat on 2026/7/27.
//

#include "app/event/sources/game_over.h"

#include "app/event/catalog.h"
#include "app/event/event_engine.h"
#include "app/service/log/log_service.h"
#include "isaac_spy/isaac/game.h"

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.event");
        return *logger;
    }
}

using namespace app::event::sources;

GameOverSource::GameOverSource(EventEngine& engine) : IEventSource(engine) {}

void GameOverSource::tick() {
    const bool is_game_over = isaac_spy::isaac::Game::get_instance().is_game_over();

    if (game_over_flag_ != is_game_over) {
        game_over_flag_ = is_game_over;

        if (is_game_over) {
            log_().info("[GameOver]");

            engine_.post(GameOverEvent{});
        }
    }
}

void GameOverSource::reset() {
    game_over_flag_ = false;
}
