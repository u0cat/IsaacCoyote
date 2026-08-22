//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_ROOM_H
#define ISAACSPY_ROOM_H
#include <cstdint>
#include <vector>

#include "entity.h"

namespace isaac_spy::isaac
{
    class Room {
    public:
        Room(uintptr_t room_ptr);

        Room(const Room&) = delete;
        Room& operator=(const Room&) = delete;
        Room(Room&&) = delete;
        Room& operator=(Room&&) = delete;

        uintptr_t get_this_ptr() { return this_ptr_; }
        std::vector<Entity> get_active_entities();

    private:
        uintptr_t this_ptr_;
    };
}
#endif //ISAACSPY_ROOM_H
