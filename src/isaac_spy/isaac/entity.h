//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_ENTITY_H
#define ISAACSPY_ENTITY_H
#include <cstdint>
#include <string>

#include "entity_config_manager.h"

namespace isaac_spy::isaac
{
    class Entity {
    public:
        Entity(uintptr_t entity_ptr);

        int get_type() const;
        int get_variant() const;
        int get_subtype() const;

        EntityConfig get_config() const;
        std::string get_localized_name() const;

        Entity get_spawner() const;
        uintptr_t get_this_ptr() const;

    private:
        mutable EntityConfig config_;

        uintptr_t this_ptr_;
    };
}

#endif //ISAACSPY_ENTITY_H
