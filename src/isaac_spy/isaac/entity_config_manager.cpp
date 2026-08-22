//
// Created by TsCat on 2026/8/9.
//

#include "isaac_spy/isaac/entity_config_manager.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

EntityConfigManager::EntityConfigManager(uintptr_t ptr): this_ptr_(ptr) {
    auto result = mem::Scanner(kPatternEntityConfigManagerGetEntity).scan();
    if (result.found) fn_get_entity = reinterpret_cast<EntityConfigM_GetEntity_t>(result.address);
}

EntityConfig EntityConfigManager::get_entity(int id, int variant, int subtype) {
    auto config = EntityConfig{};
    if (!fn_get_entity) return config;

    uintptr_t raw_config = fn_get_entity(this_ptr_, id, variant, subtype);
    if (!raw_config) return config;

    config.id = mem::read_value<int>(raw_config + kOffsetEntityConfigId);
    config.variant = mem::read_value<int>(raw_config + kOffsetEntityConfigVariant);
    config.subtype = mem::read_value<int>(raw_config + kOffsetEntityConfigSubtype);
    config.name = mem::read_string(raw_config + kOffsetEntityConfigName);

    return config;
}


