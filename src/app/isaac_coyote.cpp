//
// Created by TsCat on 2026/7/15.
//

#include "app/isaac_coyote.h"

#include "app/service/log/log_service.h"

using namespace app;

namespace
{
    spdlog::logger& log_()
    {
        static auto logger = app::log::get("app.main");
        return *logger;
    }
}

IsaacCoyote::~IsaacCoyote() {
    stop();
}

overlay::OverlayService* IsaacCoyote::get_overlay() const {
    if (!running) {
        return nullptr;
    }
    return overlay_service_.get();
}

event::EventEngine* IsaacCoyote::get_event_engine() const {
    if (!running) {
        return nullptr;
    }
    return event_engine_.get();
}

game::Game* IsaacCoyote::get_game() const {
    return running ? game_.get() : nullptr;
}

coyote::CoyoteService* IsaacCoyote::get_coyote_service() const {
    return running ? coyote_service_.get() : nullptr;
}

config::ConfigService* IsaacCoyote::get_config_service() const {
    return config_.get();
}

void IsaacCoyote::run(const std::filesystem::path& config_path) {
    if (running)
        return;

    stopped_ = false;
    log_().info("IsaacCoyote starting, config: {}", config_path.string());

    config_ = std::make_unique<config::ConfigService>(config_path);
    if (!config_->load())
        log_().warn("config load failed, using defaults");
    app::log::apply_config(config_->snapshot()->logging);

    coyote_service_ = std::make_unique<coyote::CoyoteService>(*config_);
    coyote_service_->start();

    event_engine_ = std::make_unique<event::EventEngine>();
    game_ = std::make_unique<game::Game>(*config_, *event_engine_, *coyote_service_);

    overlay_service_ = std::make_unique<overlay::OverlayService>(*game_, *config_);
    if (!overlay_service_->is_hook_installed()) {
        log_().error("overlay hook failed to install, shutting down");
        stop();
        return;
    }

    running = true;
    log_().info("IsaacCoyote running");
}

void IsaacCoyote::stop() {
    if (stopped_)
        return;
    stopped_ = true;
    log_().info("IsaacCoyote stopping");
    running = false;
    overlay_service_.reset();
    game_.reset();
    event_engine_.reset();
    if (coyote_service_) coyote_service_->stop();
    coyote_service_.reset();
    config_.reset();
    log_().info("IsaacCoyote stopped");
}

void IsaacCoyote::tick() {
    if (running && game_)
        game_->tick();
}
