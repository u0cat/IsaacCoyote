//
// Created by TsCat on 2026/7/16.
//

#ifndef ISAACCOYOTE_EVENT_ENGINE_H
#define ISAACCOYOTE_EVENT_ENGINE_H
#include <cstdint>
#include <expected>
#include <functional>
#include <queue>
#include <unordered_map>

#include "catalog.h"
#include "event_source_manager.h"


namespace app::event
{
    using EventCallback = std::function<void(EventVariant)>;

    struct CallbackEntry {
        EventType type;
        EventCallback callback;
    };

    class EventEngine {
    public:
        using CallbackId = uint64_t;

        EventEngine();
        std::expected<CallbackId, sources::EventSourceError> on(EventType type, EventCallback callback);
        bool off(CallbackId id);

        void tick();
        void dispatch_all();
        void reset();

        void post(EventVariant event);
        void dispatch(const EventVariant& event);
    private:
        std::queue<EventVariant> event_queue_;

        std::unordered_map<CallbackId, CallbackEntry> callbacks_;
        std::unordered_map<EventType, std::vector<CallbackId>> type_index_;

        EventSourceManager source_manager_;
        CallbackId next_id_ = 0;
    };
}
#endif //ISAACCOYOTE_EVENT_ENGINE_H
