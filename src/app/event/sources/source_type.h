//
// Created by TsCat on 2026/7/16.
//

#ifndef ISAACCOYOTE_SOURCE_TYPE_H
#define ISAACCOYOTE_SOURCE_TYPE_H
#include <expected>
#include <string>

namespace app::event { class EventEngine; }

namespace app::event::sources
{
    struct EventSourceError {
        std::string message;
    };

    class IEventSource {
    public:
        explicit IEventSource(EventEngine& engine) : engine_(engine) {}
        virtual ~IEventSource() = default;

        virtual std::expected<void, EventSourceError> enable() { return {}; }
        virtual void disable() {}
        virtual void tick() = 0;
        virtual void reset() = 0;

    protected:
        EventEngine& engine_;
    };
}
#endif //ISAACCOYOTE_SOURCE_TYPE_H