// Created by TsCat on 2026/7/13.
//
#include "isaac_spy/isaac/netplay_manager.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/isaac/manager.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

namespace isaac_spy::isaac
{
    std::string player_name_of(uintptr_t player_ptr)
    {
        if (!player_ptr) return {};
        auto* manager = Manager::get_instance().get_netplay_manager();
        if (!manager) return {};
        return manager->get_player_name(player_ptr).value_or("");
    }
}

NetPlayManager::NetPlayManager(uintptr_t ptr) : this_ptr_(ptr) {
    init();
}

// Only available in online game
uintptr_t NetPlayManager::get_local_player_ptr() {
    if (!fn_get_local_player) init();
    return fn_get_local_player ? fn_get_local_player(this_ptr_) : 0;
}

// Entity_Player -> device_id -> NetManager device -> Steam profile -> nickname.
std::optional<std::string> NetPlayManager::get_player_name(uintptr_t player_ptr) {
    if (!player_ptr) return std::nullopt;
    const uint32_t device_id = mem::read_value<uint32_t>(player_ptr + kOffsetPlayerDeviceId);
    return get_device_name(device_id);
}

std::optional<std::string> NetPlayManager::get_device_name(uint32_t device_id) {
    if (!fn_get_device_by_id || !fn_format_user_profile_name) init();

    const uintptr_t devices = this_ptr_;
    const uintptr_t device = fn_get_device_by_id ? fn_get_device_by_id(devices, device_id) : 0;
    if (!device) return std::nullopt;

    const uintptr_t holder = mem::read_ptr(device + kOffsetDeviceProfileHolder);
    if (!holder) return std::nullopt;

    const uint32_t low = mem::read_value<uint32_t>(holder + kOffsetProfileSteamIdLow);
    const uint32_t high = mem::read_value<uint32_t>(holder + kOffsetProfileSteamIdHigh);
    if (!low && !high) return std::nullopt;

    if (!fn_format_user_profile_name) return std::nullopt;
    const auto* manager = &Manager::get_instance();

    const char* name = fn_format_user_profile_name(
        low, high, manager->get_this_ptr() + kOffsetManagerProfileNameFormat, 0x7D);
    if (!name) return std::nullopt;

    return std::string{name};
}

void NetPlayManager::init() {
    auto result = mem::Scanner(kPatternNetPlayManagerGetLocalPlayer).scan();
    if (result.found) fn_get_local_player = reinterpret_cast<NetPlayManager_GetLocalPlayer_t>(result.address);

    result = mem::Scanner(kPatternNetManagerGetDeviceById).scan();
    if (result.found) fn_get_device_by_id = reinterpret_cast<NetManager_GetDeviceById_t>(result.address);

    result = mem::Scanner(kPatternNetManagerFormatUserProfileName).scan();
    if (result.found) fn_format_user_profile_name = reinterpret_cast<NetManager_FormatUserProfileName_t>(result.address);
}
