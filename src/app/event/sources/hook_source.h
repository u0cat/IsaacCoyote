//
// Created by TsCat on 2026/8/20.
//

#ifndef ISAACCOYOTE_HOOK_SOURCE_H
#define ISAACCOYOTE_HOOK_SOURCE_H
#include <expected>
#include <queue>
#include <string_view>

#include "app/event/event_engine.h"
#include "app/event/sources/source_type.h"
#include "app/service/log/log_service.h"
#include "isaac_spy/isaac/game.h"

namespace app::event::sources
{
    // Hook-backed source skeleton: subscribe on enable, drain on tick, post the event.
    // Subclasses supply the per-event differences (accept / make_event / log_event).
    template <typename Ctx, typename Event, typename Hook>
    class HookSource : public IEventSource {
    public:
        HookSource(EventEngine& engine, Hook& hook, std::string_view log_tag)
            : IEventSource(engine), hook_(hook), log_tag_(log_tag) {}

        std::expected<void, EventSourceError> enable() override {
            auto result = hook_.subscribe([this](Ctx context) { enqueue(std::move(context)); });
            if (!result) return std::unexpected(EventSourceError{result.error().message});
            subscription_ = *result;
            return {};
        }

        void disable() override {
            hook_.unsubscribe(subscription_);
            subscription_ = 0;
            reset();
        }

        void tick() override {
            std::queue<Ctx> pending;
            pending.swap(pending_);

            auto* player_manager = isaac_spy::isaac::Game::get_instance().get_player_manager();
            if (!player_manager) {
                if (!pending.empty()) logger_().debug("[{}] dropped: player manager unavailable", log_tag_);
                return;
            }

            while (!pending.empty()) {
                auto details = std::move(pending.front());
                pending.pop();

                if (!accept(details)) continue;

                if (!player_manager->find_player(details.player)) {
                    logger_().debug("[{}] dropped: player {:#x} is not registered", log_tag_, details.player);
                    continue;
                }

                const event::PlayerRelation relation = player_manager->is_local_player(details.player)
                                                           ? event::PlayerRelation::Self
                                                           : event::PlayerRelation::Other;
                log_event(details, relation);

                event::PlayerId player_id = event::ptr_player_id(details.player);
                Event event = make_event(std::move(details));
                event.context = event::EventContext{std::move(player_id), relation};

                engine_.post(std::move(event));
            }
        }

        void reset() override {
            pending_ = {};
        }

    protected:
        virtual bool accept(const Ctx&) { return true; }

        // Payload without context; Ctx must expose `player`.
        virtual Event make_event(Ctx&& ctx) = 0;

        virtual void log_event(const Ctx&, event::PlayerRelation) {}

        static std::string_view relation_name(event::PlayerRelation rel) {
            return rel == event::PlayerRelation::Self ? "Self" : "Other";
        }

    private:
        void enqueue(Ctx context) {
            pending_.push(std::move(context));
        }

        spdlog::logger& logger_() const {
            static auto logger = app::log::get("app.event");
            return *logger;
        }

        Hook& hook_;
        std::string_view log_tag_;
        typename Hook::Subscription subscription_ = 0;
        std::queue<Ctx> pending_;
    };
}
#endif //ISAACCOYOTE_HOOK_SOURCE_H