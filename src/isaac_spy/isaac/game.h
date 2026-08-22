//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_GAME_H
#define ISAACSPY_GAME_H
#include <memory>

#include "item_pool.h"
#include "player_manager.h"
#include "room.h"

namespace isaac_spy::isaac
{
    typedef int (__thiscall *Game_GetNumPlayers_t)(uintptr_t self);
    typedef int (__thiscall *Game_IsPaused_t)(uintptr_t self);
    typedef void (__thiscall *Game_ScreenShake_t)(uintptr_t self, int duration);

    class Game {
    public:
        static Game& get_instance() {
            static Game instance;
            if (!instance.is_initialized) instance.init();
            return instance;
        }

        PlayerManager* get_player_manager();

        bool is_paused();
        bool is_game_over();

        void shake_screen(int extent);

        Room* get_current_room();
        int get_current_room_index();

        ItemPool* get_item_pool();

        uintptr_t get_this_ptr();

    private:
        Game() = default;

        uintptr_t this_ptr_ = 0;

        bool is_initialized = false;
        void init();

        std::unique_ptr<PlayerManager> player_manager_ = nullptr;
        std::unique_ptr<Room> room_ = nullptr;
        std::unique_ptr<ItemPool> item_pool_ = nullptr;

        Game_IsPaused_t fn_is_paused = nullptr;
        Game_ScreenShake_t fn_screen_shake = nullptr;
    };
}

#endif //ISAACSPY_GAME_H
