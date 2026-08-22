//
// Created by TsCat on 2026/8/8.
//

#include "isaac_spy/isaac/entity_ref.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

EntityRef::EntityRef(uintptr_t entity_ref_ptr) : this_ptr_(entity_ref_ptr), entity_ptr_(mem::read_ptr(this_ptr_ + kOffsetEntityRefEntity)) {}

int EntityRef::get_type() const {
    return get_entity().get_type();
}

int EntityRef::get_variant() const {
    return get_entity().get_variant();
}

int EntityRef::get_subtype() const {
    return get_entity().get_subtype();
}

Entity EntityRef::get_entity() const {
    if (!entity_ptr_) {
        entity_ptr_ = mem::read_ptr(this_ptr_ + kOffsetEntityRefEntity);
    }
    return Entity(entity_ptr_);
}




