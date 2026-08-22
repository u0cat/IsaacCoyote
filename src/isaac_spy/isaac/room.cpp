//
// Created by TsCat on 2026/7/11.
//

#include "isaac_spy/isaac/room.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

Room::Room(uintptr_t room_ptr) : this_ptr_(room_ptr) {}

std::vector<Entity> Room::get_active_entities() {
    std::vector<Entity> active_entities;

    uintptr_t entity_list_ptr = this_ptr_ + kOffsetRoomEntityList;
    uintptr_t active_entity_list = entity_list_ptr + kOffsetRoomActiveEntityList;

    uintptr_t begin = mem::read_ptr(active_entity_list + kOffsetRoomListBegin);
    size_t size = mem::read_ptr(active_entity_list + kOffsetRoomListSize);

    for (uint32_t i = 0; i < size; ++i) {
        const uintptr_t entity_ptr = mem::read_ptr(begin + i * 4);
        if (entity_ptr == 0) continue;

        active_entities.emplace_back(entity_ptr);
    }

    return active_entities;
};
