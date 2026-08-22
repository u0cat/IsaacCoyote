//
// Created by TsCat on 2026/7/25.
//

#ifndef ISAACCOYOTE_REROLL_SOURCE_H
#define ISAACCOYOTE_REROLL_SOURCE_H
#include "isaac_spy/hook_manager.h"
#include "isaac_spy/hooks/restart_game_hook.h"
#include "isaac_spy/hooks/start_new_game_hook.h"
#include "source_type.h"

namespace app::event::sources
{
    class RerollSource final : public IEventSource {
    public:
        RerollSource(EventEngine& engine, isaac_spy::hook::HookManager& hook_manager);

        std::expected<void, EventSourceError> enable() override;
        void disable() override;
        void tick() override;
        void reset() override;

    private:
        isaac_spy::hook::StartNewGameHook& start_new_game_hook_;
        isaac_spy::hook::RestartGameHook& restart_game_hook_;
        isaac_spy::hook::StartNewGameHook::Subscription start_subscription_ = 0;
        isaac_spy::hook::RestartGameHook::Subscription restart_subscription_ = 0;

        bool can_continue_ = false;
        bool new_game_flag_ = false;
        bool restart_flag_ = false;
    };
}
#endif //ISAACCOYOTE_REROLL_SOURCE_H
