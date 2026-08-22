//
// Created by TsCat on 2026/7/12.
//

#include "isaac_spy/isaac/item_pool.h"

#include <cinttypes>

#include "isaac_spy/game_constants.h"
#include "isaac_spy/isaac/player.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

ItemPool::ItemPool(uintptr_t ptr) : this_ptr_(ptr) {}

int ItemPool::get_pill_effect(int pill_color, const Player& player) {
    if (!initialized) init();
    if (fn_get_pill_effect) {
        return fn_get_pill_effect(this_ptr_, pill_color, player.get_this_ptr());
    }

    return -1;
}

void ItemPool::init() {
    auto result = mem::Scanner(kPatternItemPoolGetPillEffect).scan();
    if (result.found) fn_get_pill_effect = reinterpret_cast<ItemPool_GetPillEffect_t>(result.address);

    if (fn_get_pill_effect != nullptr)initialized = true;
}
