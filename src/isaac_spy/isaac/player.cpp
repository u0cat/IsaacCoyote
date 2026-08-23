//
// Created by TsCat on 2026/7/10.
//

#include "isaac_spy/isaac/player.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/isaac/game.h"
#include "isaac_spy/isaac/trinket.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/memory_ref.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

Player::Player(uintptr_t player_ptr)
    : max_hearts_(player_ptr + kOffsetPlayerMaxHearts),
      red_hearts_(player_ptr + kOffsetPlayerRedHearts),
      eternal_hearts_(player_ptr + kOffsetPlayerEternalHearts),
      soul_hearts_(player_ptr + kOffsetPlayerSoulHearts),
      black_hearts_(player_ptr + kOffsetPlayerBlackHearts),
      keys_(player_ptr + kOffsetPlayerKeys),
      bombs_(player_ptr + kOffsetPlayerBombs),
      coins_(player_ptr + kOffsetPlayerCoins),
      player_type_(player_ptr + kOffsetPlayerPlayerType),
      can_fly_(player_ptr + kOffsetPlayerCanFly),
      device_id_(player_ptr + kOffsetPlayerDeviceId),
      this_ptr_(player_ptr) {}

Player* Player::get_twin() {
    auto* player_manager = Game::get_instance().get_player_manager();
    if (player_manager) return player_manager->get_twin(*this);
    return nullptr;
}

std::unordered_map<int, int> Player::get_collectibles() {
    uintptr_t begin = mem::read_ptr(this_ptr_ + kOffsetPlayerCollectibles);
    uintptr_t end = mem::read_ptr(this_ptr_ + kOffsetPlayerCollectiblesEnd);

    if (begin == 0 || end <= begin) {
        return {};
    }

    std::unordered_map<int, int> result;

    for (uintptr_t addr = begin; addr < end; addr += 4) {
        if (int num = mem::read_value<int>(addr); num > 0) {
            int index = (addr - begin) / 4;
            result[index] = num;
        }
    }

    return result;
}

std::vector<PocketItemDesc> Player::get_pocket_items() {
    std::vector<PocketItemDesc> pocket_items;
    pocket_items.reserve(3);

    auto* item_pool = Game::get_instance().get_item_pool();

    for (int i = 0; i < 3; ++i) {
        uintptr_t item_ptr = this_ptr_ + kOffsetPlayerPocketItems + i * 8;
        int id = mem::read_value<int>(item_ptr);
        int type = mem::read_value<int>(item_ptr + 0x4);

        int actual_id = (type == 0)
                            ? item_pool->get_pill_effect(id, *this)
                            : id;

        pocket_items.emplace_back(actual_id, type);
    }

    return pocket_items;
}

std::vector<TrinketDesc> Player::get_trinkets() {
    const int slot1 = mem::read_value<int>(this_ptr_ + kOffsetPlayerTrinkets);
    const int slot2 = mem::read_value<int>(this_ptr_ + kOffsetPlayerTrinkets + 4);

    std::vector<TrinketDesc> trinkets;
    trinkets.reserve(2);

    if (slot1 > 0) {
        trinkets.emplace_back(slot1);
    }
    if (slot2 > 0) {
        trinkets.emplace_back(slot2);
    }
    return trinkets;
}
