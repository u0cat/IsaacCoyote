//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_PLAYER_H
#define ISAACSPY_PLAYER_H

#include <unordered_map>
#include <vector>

#include "isaac_spy/memory_ref.h"
#include "pocket_item.h"
#include "trinket.h"

namespace isaac_spy::isaac
{
    class Player {
    public:
        Player(uintptr_t player_ptr);

        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;
        Player(Player&&) = delete;
        Player& operator=(Player&&) = delete;

        mem::MemoryRef<int>& get_max_hearts() { return max_hearts_; }
        mem::MemoryRef<int>& get_red_hearts() { return red_hearts_; }
        mem::MemoryRef<int>& get_eternal_hearts() { return eternal_hearts_; }
        mem::MemoryRef<int>& get_soul_hearts() { return soul_hearts_; }
        mem::MemoryRef<int>& get_black_hearts() { return black_hearts_; }
        mem::MemoryRef<int>& get_keys() { return keys_; }
        mem::MemoryRef<int>& get_bombs() { return bombs_; }
        mem::MemoryRef<int>& get_coins() { return coins_; }
        mem::MemoryRef<int>& get_player_type() { return player_type_; }
        mem::MemoryRef<bool>& can_fly() { return can_fly_; }
        mem::MemoryRef<uint32_t>& get_device_id() { return device_id_; }

        Player* get_twin();
        std::unordered_map<int, int> get_collectibles();
        std::vector<PocketItemDesc> get_pocket_items();
        std::vector<TrinketDesc> get_trinkets();

        uintptr_t get_this_ptr() const { return this_ptr_; }

    private:
        mem::MemoryRef<int> max_hearts_;
        mem::MemoryRef<int> red_hearts_;
        mem::MemoryRef<int> eternal_hearts_;
        mem::MemoryRef<int> soul_hearts_;
        mem::MemoryRef<int> black_hearts_;
        mem::MemoryRef<int> keys_;
        mem::MemoryRef<int> bombs_;
        mem::MemoryRef<int> coins_;
        mem::MemoryRef<int> player_type_;
        mem::MemoryRef<bool> can_fly_;
        mem::MemoryRef<uint32_t> device_id_;

        uintptr_t this_ptr_;
    };
}
#endif //ISAACSPY_PLAYER_H
