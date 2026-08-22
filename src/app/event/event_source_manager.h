//
// Created by TsCat on 2026/7/16.
//

#ifndef ISAACCOYOTE_EVENT_SOURCE_MANAGER_H
#define ISAACCOYOTE_EVENT_SOURCE_MANAGER_H
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "catalog.h"
#include "isaac_spy/hook_manager.h"
#include "sources/source_type.h"

namespace app::event
{
    class EventEngine;

    class EventSourceManager {
    public:
        explicit EventSourceManager(EventEngine& engine);

        template <typename T, typename... Args>
        bool register_source(EventType type, Args&&... args) {
            if (sources_.contains(type))
                return false;

            sources_.emplace(
                type,
                std::make_unique<T>(engine_, std::forward<Args>(args)...)
            );
            return true;
        }

        std::expected<void, sources::EventSourceError> enable(EventType type);
        bool disable(EventType type);
        void tick();
        void reset();
        isaac_spy::hook::HookManager& get_hook_manager();

    private:
        EventEngine& engine_;
        isaac_spy::hook::HookManager hook_manager_;

        std::vector<EventType> enabled_;
        std::unordered_map<EventType, std::unique_ptr<sources::IEventSource>> sources_;
    };
}
#endif //ISAACCOYOTE_EVENT_SOURCE_MANAGER_H
