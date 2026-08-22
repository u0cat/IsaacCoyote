//
// Created by TsCat on 2026/7/13.
//

#ifndef ISAACSPY_NETPLAY_MANAGER_H
#define ISAACSPY_NETPLAY_MANAGER_H
#include <cstdint>

namespace isaac_spy::isaac
{

    typedef uintptr_t (__thiscall *NetPlayManager_GetLocalPlayer_t)(uintptr_t self);

    class NetPlayManager {
    public:
        NetPlayManager(uintptr_t ptr);
        uintptr_t get_local_player_ptr();
        // bool is_idx_net_player();
        // Player get_remote_player(int index);

    private:
        void init();

        NetPlayManager_GetLocalPlayer_t fn_get_local_player = nullptr;

        uintptr_t this_ptr_;
    };
}
#endif //ISAACSPY_NETPLAY_MANAGER_H
