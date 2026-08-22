//
// Created by TsCat on 2026/7/10.
//

#include "isaac_spy/isaac/player_manager.h"

#include <array>
#include <cinttypes>
#include <unordered_set>

#include "isaac_spy/game_constants.h"
#include "isaac_spy/isaac/manager.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

PlayerManager::PlayerManager(uintptr_t player_manager_ptr) {
    this_ptr_ = player_manager_ptr;

    auto result = mem::Scanner(kPatternPlayerManagerIsMultiPlay).scan();
    if (result.found) fn_is_multi_play_ = reinterpret_cast<PlayerManager_IsMultiPlay_t>(result.address);
}

std::vector<Player*> PlayerManager::get_player_list() {
    update_players();

    std::vector<Player*> result;
    result.reserve(player_ptrs_.size());

    for (const uintptr_t player_ptr : player_ptrs_) {
        const auto it = players_.find(player_ptr);
        if (it != players_.end()) {
            result.push_back(it->second.get());
        }
    }

    return result;
}

Player* PlayerManager::get_player(int index) {
    update_players();

    if (index < 0) {
        return nullptr;
    }

    const size_t player_index = static_cast<size_t>(index);
    if (player_index >= player_ptrs_.size()) {
        return nullptr;
    }

    const uintptr_t player_ptr = player_ptrs_[player_index];
    const auto it = players_.find(player_ptr);

    return it != players_.end() ? it->second.get() : nullptr;
}

Player* PlayerManager::get_local_player() {
    const auto player_list = get_player_list();

    uintptr_t player_ptr = 0;
    if (auto* netplay = Manager::get_instance().get_netplay_manager())
        player_ptr = netplay->get_local_player_ptr();

    if (player_ptr) {
        const auto it = players_.find(player_ptr);
        if (it != players_.end()) return it->second.get();
    }

    return (!player_list.empty() && player_list.front()) ? player_list.front() : nullptr;
}

Player* PlayerManager::find_player(uintptr_t player_ptr) {
    update_players();

    const auto it = players_.find(player_ptr);
    return it != players_.end() ? it->second.get() : nullptr;
}

Player* PlayerManager::get_twin(const Player& player) {
    uintptr_t twin_ptr = mem::read_ptr(player.get_this_ptr() + kOffsetPlayerManagerTwin);
    if (!twin_ptr) return nullptr;

    const auto it = players_.find(twin_ptr);
    return it != players_.end() ? it->second.get() : nullptr;
}

bool PlayerManager::is_local_player(uintptr_t player_ptr) {
    Player* local = get_local_player();
    if (!local) return false;
    if (local->get_this_ptr() == player_ptr) return true;
    if (Player* twin = get_twin(*local))
        return twin->get_this_ptr() == player_ptr;
    return false;
}

int PlayerManager::get_player_num() {
    update_players();
    return static_cast<int>(player_ptrs_.size());
}

bool PlayerManager::is_multi_play() {
    if (fn_is_multi_play_) {
        return fn_is_multi_play_(this_ptr_);
    }
    return false;
}

uintptr_t PlayerManager::get_this_ptr() {
    return this_ptr_;
}

void PlayerManager::update_players() {
    constexpr size_t kMaxPlayers = 16;

    std::array<uintptr_t, 2> bounds{};
    if (!mem::safe_read_raw(bounds.data(), this_ptr_, sizeof(bounds)))
        return;

    const uintptr_t begin = bounds[0];
    const uintptr_t end = bounds[1];

    if (begin == end) {
        player_ptrs_.clear();
        players_.clear();
        return;
    }

    if (!begin || end < begin)
        return;

    const uintptr_t byte_size = end - begin;
    if (byte_size % sizeof(uintptr_t) != 0)
        return;

    const size_t count = byte_size / sizeof(uintptr_t);
    if (count > kMaxPlayers)
        return;

    std::vector<uintptr_t> result(count);
    if (!mem::safe_read_raw(result.data(), begin, byte_size))
        return;

    if (result == player_ptrs_)
        return;

    std::unordered_set<uintptr_t> live_players;
    for (uintptr_t ptr : result) {
        if (ptr)
            live_players.insert(ptr);
    }

    std::erase_if(players_, [&](const auto& kv)
    {
        return !live_players.contains(kv.first);
    });

    for (uintptr_t ptr : result) {
        if (ptr && !players_.contains(ptr)) {
            players_.emplace(ptr, std::make_unique<Player>(ptr));
        }
    }

    player_ptrs_ = std::move(result);
}
