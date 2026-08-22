//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_PLAYER_MANAGER_H
#define ISAACSPY_PLAYER_MANAGER_H
#include <memory>
#include <unordered_map>
#include <vector>

#include "player.h"

namespace isaac_spy::isaac
{
    typedef bool (__thiscall *PlayerManager_IsMultiPlay_t)(uintptr_t self);

    class PlayerManager {
    public:
        PlayerManager(uintptr_t player_manager_ptr);

        PlayerManager(const PlayerManager&) = delete;
        PlayerManager& operator=(const PlayerManager&) = delete;
        PlayerManager(PlayerManager&&) = delete;
        PlayerManager& operator=(PlayerManager&&) = delete;

        std::vector<Player*> get_player_list();
        Player* get_player(int index);
        Player* get_local_player();
        Player* find_player(uintptr_t player_ptr);
        Player* get_twin(const Player& player);
        bool is_local_player(uintptr_t player_ptr);
        int get_player_num();
        bool is_multi_play();

        uintptr_t get_this_ptr();

    private:
        uintptr_t this_ptr_;

        void update_players();
        std::vector<uintptr_t> player_ptrs_;
        std::unordered_map<uintptr_t, std::unique_ptr<Player>> players_;

        PlayerManager_IsMultiPlay_t fn_is_multi_play_ = nullptr;
    };
}
#endif //ISAACSPY_PLAYER_MANAGER_H
