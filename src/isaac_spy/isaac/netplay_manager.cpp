//
// Created by TsCat on 2026/7/13.
//
#include "isaac_spy/isaac/netplay_manager.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

NetPlayManager::NetPlayManager(uintptr_t ptr) : this_ptr_(ptr) {
    init();
}

// Only available in online game
uintptr_t NetPlayManager::get_local_player_ptr() {
    if (!fn_get_local_player) init();
    return fn_get_local_player ? fn_get_local_player(this_ptr_) : 0;
}

void NetPlayManager::init() {
    auto result = mem::Scanner(kPatternNetPlayManagerGetLocalPlayer).scan();
    if (result.found) fn_get_local_player = reinterpret_cast<NetPlayManager_GetLocalPlayer_t>(result.address);
}
