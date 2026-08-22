//
// Created by TsCat on 2026/7/11.
//

#include "isaac_spy/isaac/manager.h"

#include <cinttypes>

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

ItemConfigManager* Manager::get_item_config_manager() {
    if (!is_initialized) return nullptr;
    if (!item_config_manager_) {
        item_config_manager_ = std::make_unique<ItemConfigManager>(this_ptr_ + kOffsetManagerItemConfigManager);
    }
    return item_config_manager_.get();
}

EntityConfigManager* Manager::get_entity_config_manager() {
    if (!is_initialized) return nullptr;
    if (!entity_config_manager_) {
        entity_config_manager_ = std::make_unique<EntityConfigManager>(this_ptr_ + kOffsetManagerEntityConfigManager);
    }
    return entity_config_manager_.get();
}

StringTable* Manager::get_string_table() {
    if (!is_initialized) return nullptr;
    if (!string_table_) {
        string_table_ = std::make_unique<StringTable>(this_ptr_ + kOffsetManagerStringTable);
    }

    return string_table_.get();
}

NetPlayManager* Manager::get_netplay_manager() {
    if (!is_initialized) return nullptr;
    if (!netplay_manager_) netplay_manager_ = std::make_unique<NetPlayManager>(this_ptr_ + kOffsetManagerNetPlayManager);

    return netplay_manager_.get();
}

bool Manager::is_in_game() const {
    if (!is_initialized) return false;
    return mem::read_value<int>(this_ptr_ + kOffsetManagerGameState) == 2;
}

bool Manager::can_continue() const {
    return mem::read_value<uint8_t>(this_ptr_ + kOffsetManagerContinueState) != 0;
}

bool Manager::can_rerun() const {
    return mem::read_value<uint8_t>(this_ptr_ + kOffsetManagerRerunState) != 0;
}

uintptr_t Manager::get_this_ptr() const {
    return this_ptr_;
}

void Manager::init() {
    auto result = mem::Scanner(kPatternManagerSingleton).scan();
    if (result.found) {
        if (!result.captures.empty()) {
            this_ptr_ = mem::read_ptr(reinterpret_cast<uintptr_t>(result.captures.front().value));
        }
    }
    else return;

    is_initialized = true;
}
