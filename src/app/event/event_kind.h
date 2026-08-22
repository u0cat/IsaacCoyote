//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_EVENT_KIND_H
#define ISAACCOYOTE_EVENT_KIND_H

namespace app::event
{
    // Single registry of all event kinds; variants, metadata and registration
    // derive from the type list below.
    enum class EventType {
        Hurt,
        Death,
        RerollGame,
        GameOver,
        UseActiveItem,
        UsePill,
        UseCard,
        None // Placeholder for no event
    };

    struct NoneEventConfig {};

    template <EventType T>
    struct EventDescriptor;

    class EventSourceManager;

    template <EventType... Ts>
    struct EventTypeList {
        template <typename F>
        static constexpr void for_each(F&& f) { (f.template operator()<Ts>(), ...); }
    };

    using AllEventTypes = EventTypeList<
        EventType::Hurt,
        EventType::Death,
        EventType::RerollGame,
        EventType::GameOver,
        EventType::UseActiveItem,
        EventType::UsePill,
        EventType::UseCard
    >;
}

#endif //ISAACCOYOTE_EVENT_KIND_H