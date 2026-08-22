//
// Created by TsCat on 2026/7/16.
//

#include "app/event/event_source_manager.h"

using namespace app::event;

EventSourceManager::EventSourceManager(EventEngine& engine) : engine_(engine) {}

std::expected<void, sources::EventSourceError> EventSourceManager::enable(EventType type) {
    const auto it = sources_.find(type);
    if (it == sources_.end()) {
        return std::unexpected(sources::EventSourceError{"event source is not registered"});
    }
    if (std::ranges::find(enabled_, type) != enabled_.end()) return {};

    auto result = it->second->enable();
    if (!result) return result;
    enabled_.push_back(type);
    return {};
}

bool EventSourceManager::disable(EventType type) {
    if (std::erase(enabled_, type) == 0) return false;
    if (const auto it = sources_.find(type); it != sources_.end()) it->second->disable();
    return true;
}

void EventSourceManager::reset() {
    for (const auto& [type, source] : sources_) {
        source->reset();
    }
}

isaac_spy::hook::HookManager& EventSourceManager::get_hook_manager() {
    return hook_manager_;
}

void EventSourceManager::tick() {
    for (auto type : enabled_) {
        if (auto it = sources_.find(type); it != sources_.end())
            it->second->tick();
    }
}
