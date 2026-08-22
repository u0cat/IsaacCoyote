//
// Created by TsCat on 2026/7/10.
//
#include "isaac_spy/isaac//entity.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/isaac/manager.h"
#include "isaac_spy/memory.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

Entity::Entity(uintptr_t entity_ptr) : this_ptr_(entity_ptr) {}

int Entity::get_type() const {
    if (this_ptr_ == 0) {
        return -1;
    }
    return mem::read_value<int>(this_ptr_ + kOffsetEntityType);
}

int Entity::get_variant() const {
    if (this_ptr_ == 0) {
        return -1;
    }
    return mem::read_value<int>(this_ptr_ + kOffsetEntityVariant);
}

int Entity::get_subtype() const {
    if (this_ptr_ == 0) {
        return -1;
    }
    return mem::read_value<int>(this_ptr_ + kOffsetEntitySubType);
}

EntityConfig Entity::get_config() const {
    if (config_.id > 0) return config_;

    auto& manager = Manager::get_instance();
    auto* config_m = manager.get_entity_config_manager();
    if (!config_m) return {};

    const auto config = config_m->get_entity(get_type(), get_variant(), get_subtype());
    if (config.id <= 0) return {};

    config_ = config;
    return config;
}

std::string Entity::get_localized_name() const {
    const auto config = get_config();

    std::string name = config.name;
    auto& manager = Manager::get_instance();
    auto* strings = manager.get_string_table();
    if (strings && name.size() > 1 && name.front() == '#') {
        const char* key = name.c_str() + 1;
        bool ok = false;
        name = strings->get_string("Entities", isaac_spy::isaac::LANGUAGE_CHINESE, key, ok);
        if (!ok || name.empty()) {
            ok = false;
            name = strings->get_string("Entities", isaac_spy::isaac::LANGUAGE_ENGLISH, key, ok);
        }
        if (!ok || name.empty()) name = config.name;
    }
    return name;
}

Entity Entity::get_spawner() const {
    if (this_ptr_ == 0) return Entity{0};
    auto parent_ptr = mem::read_value<uintptr_t>(this_ptr_ + kOffsetEntitySpawner);
    if (parent_ptr == 0) return Entity{0};
    return Entity{parent_ptr};
}

uintptr_t Entity::get_this_ptr() const {
    return this_ptr_;
}

