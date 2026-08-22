//
// Created by TsCat on 2026/7/16.
//

#include "app/event/event_engine.h"

#include "app/event/catalog.h"

using namespace app::event;

EventEngine::EventEngine() : source_manager_(*this) {
    for_each_event_type([&]<EventType T>()
    {
        EventDescriptor<T>::register_source(source_manager_);
    });
}

std::expected<EventEngine::CallbackId, sources::EventSourceError> EventEngine::on(
    EventType type, EventCallback callback
) {
    if (auto result = source_manager_.enable(type); !result)
        return std::unexpected(result.error());

    CallbackId id = next_id_++;
    callbacks_.emplace(id, CallbackEntry{type, std::move(callback)});
    type_index_[type].push_back(id);

    return id;
}

bool EventEngine::off(CallbackId id) {
    auto node = callbacks_.extract(id);
    if (node.empty())
        return false;

    const EventType type = node.mapped().type;
    auto it = type_index_.find(type);
    if (it == type_index_.end())
        return true;

    auto& ids = it->second;
    std::erase(ids, id);

    if (ids.empty()) {
        type_index_.erase(it);
        source_manager_.disable(type);
    }

    return true;
}

void EventEngine::tick() {
    source_manager_.tick();
    dispatch_all();
}

void EventEngine::dispatch_all() {
    std::queue<EventVariant> pending;
    pending.swap(event_queue_);

    while (!pending.empty()) {
        auto event = std::move(pending.front());
        pending.pop();
        dispatch(event);
    }
}

void EventEngine::reset() {
    source_manager_.reset();
    event_queue_ = {};
}

void EventEngine::post(EventVariant event) {
    event_queue_.push(std::move(event));
}

void EventEngine::dispatch(const EventVariant& event) {
    EventType type = event_type_of(event);

    const auto it = type_index_.find(type);
    if (it == type_index_.end()) return;

    for (const CallbackId id : it->second) {
        if (const auto callback = callbacks_.find(id); callback != callbacks_.end())
            callback->second.callback(event);
    }
}
