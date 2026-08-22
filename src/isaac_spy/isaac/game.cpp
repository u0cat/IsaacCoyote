//
// Created by TsCat on 2026/7/10.
//
#include "isaac_spy/isaac/game.h"

#include <cinttypes>

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

PlayerManager* Game::get_player_manager() {
    if (!is_initialized) return nullptr;
    if (!player_manager_) {
        player_manager_ = std::make_unique<PlayerManager>(this_ptr_ + kOffsetGamePlayerManager);
    }

    return player_manager_.get();
}

bool Game::is_game_over() {
    return mem::read_value<int>(this_ptr_ + kOffsetGameGameOverState) == 2;
}

bool Game::is_paused() {
    if (fn_is_paused) return fn_is_paused(this_ptr_);
    return false;
}

void Game::shake_screen(int extent) {
    if (fn_screen_shake) return fn_screen_shake(this_ptr_, extent);
}

Room* Game::get_current_room() {
    if (!is_initialized) return nullptr;
    if (!room_) room_ = std::make_unique<Room>(this_ptr_ + kOffsetGameCurrentRoom);

    return room_.get();
}

int Game::get_current_room_index() {
    return mem::read_value<int>(this_ptr_ + kOffsetGameCurrentRoomIndex);
}

ItemPool* Game::get_item_pool() {
    if (!is_initialized) return nullptr;
    if (!item_pool_) {
        item_pool_ = std::make_unique<ItemPool>(this_ptr_ + kOffsetGameItemPool);
    }

    return item_pool_.get();
}

uintptr_t Game::get_this_ptr() {
    return this_ptr_;
}

void Game::init() {
    if (is_initialized) return;

    auto result = mem::Scanner(kPatternGameSingleton).scan();
    if (result.found) {
        if (!result.captures.empty()) {
            this_ptr_ = mem::read_ptr(reinterpret_cast<uintptr_t>(result.captures.front().value));
        }
    }
    else return;

    result = mem::Scanner(kPatternGameIsPaused).scan();
    if (result.found) fn_is_paused = reinterpret_cast<Game_IsPaused_t>(result.address);

    result = mem::Scanner(kPatternGameScreenShake).scan();
    if (result.found) fn_screen_shake = reinterpret_cast<Game_ScreenShake_t>(result.address);

    is_initialized = true;
}
