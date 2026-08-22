//
// Created by TsCat on 2026/7/15.
//

#ifndef ISAACCOYOTE_ISAAC_COYOTE_H
#define ISAACCOYOTE_ISAAC_COYOTE_H
#include <memory>

#include "app/service/config/config_service.h"
#include "app/service/overlay/overlay_service.h"
#include "event/event_engine.h"
#include "game.h"
#include "service/coyote/coyote_service.h"

namespace app
{
    class IsaacCoyote {
    public:
        ~IsaacCoyote();

        IsaacCoyote(const IsaacCoyote&) = delete;
        IsaacCoyote& operator=(const IsaacCoyote&) = delete;
        IsaacCoyote(IsaacCoyote&&) = delete;
        IsaacCoyote& operator=(IsaacCoyote&&) = delete;

        static IsaacCoyote& get_instance() {
            static IsaacCoyote instance;
            return instance;
        }

        overlay::OverlayService* get_overlay() const;
        event::EventEngine* get_event_engine() const;
        game::Game* get_game() const;
        coyote::CoyoteService* get_coyote_service() const;
        config::ConfigService* get_config_service() const;

        void run(const std::filesystem::path& config_path = "isaac-coyote.json");
        void stop();
        void tick();

    private:
        IsaacCoyote() = default;

        bool running = false;
        bool stopped_ = true;

        std::unique_ptr<overlay::OverlayService> overlay_service_ = nullptr;
        std::unique_ptr<config::ConfigService> config_ = nullptr;

        std::unique_ptr<event::EventEngine> event_engine_ = nullptr;
        std::unique_ptr<game::Game> game_ = nullptr;
        std::unique_ptr<coyote::CoyoteService> coyote_service_ = nullptr;
    };
}
#endif //ISAACCOYOTE_ISAAC_COYOTE_H
