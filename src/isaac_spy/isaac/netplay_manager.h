//
// Created by TsCat on 2026/7/13.
//

#ifndef ISAACSPY_NETPLAY_MANAGER_H
#define ISAACSPY_NETPLAY_MANAGER_H
#include <cstdint>
#include <optional>
#include <string>

namespace isaac_spy::isaac
{

    typedef uintptr_t (__thiscall *NetPlayManager_GetLocalPlayer_t)(uintptr_t self);
    typedef uintptr_t (__thiscall *NetManager_GetDeviceById_t)(uintptr_t devices, uint32_t device_id);
    typedef const char* (__stdcall *NetManager_FormatUserProfileName_t)(uint32_t low, uint32_t high,
                                                                        uintptr_t string_table, int max_len);

    std::string player_name_of(uintptr_t player_ptr);

    class NetPlayManager {
    public:
        NetPlayManager(uintptr_t ptr);
        uintptr_t get_local_player_ptr();
        std::optional<std::string> get_player_name(uintptr_t player_ptr);
        std::optional<std::string> get_device_name(uint32_t device_id);

    private:
        void init();

        NetPlayManager_GetLocalPlayer_t fn_get_local_player = nullptr;
        NetManager_GetDeviceById_t fn_get_device_by_id = nullptr;
        NetManager_FormatUserProfileName_t fn_format_user_profile_name = nullptr;

        uintptr_t this_ptr_;
    };
}
#endif //ISAACSPY_NETPLAY_MANAGER_H
