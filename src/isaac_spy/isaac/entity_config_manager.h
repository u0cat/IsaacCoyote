//
// Created by TsCat on 2026/8/9.
//

#ifndef ISAACCOYOTE_ENTITY_CONFIG_MANAGER_H
#define ISAACCOYOTE_ENTITY_CONFIG_MANAGER_H
#include <cstdint>
#include <string>

namespace isaac_spy::isaac
{
    typedef int (__thiscall *EntityConfigM_GetEntity_t)(uintptr_t self, int id, int variant, int subtype);

    struct EntityConfig {
        int id = 0;
        int variant = 0;
        int subtype = 0;
        std::string name = "";
    };

    class EntityConfigManager {
    public:
        explicit EntityConfigManager(uintptr_t ptr);
        ~EntityConfigManager() = default;

        EntityConfigManager(const EntityConfigManager&) = delete;
        EntityConfigManager& operator=(const EntityConfigManager&) = delete;
        EntityConfigManager(EntityConfigManager&&) noexcept = default;
        EntityConfigManager& operator=(EntityConfigManager&&) noexcept = default;

        EntityConfig get_entity(int id, int variant, int subtype);
    private:
        uintptr_t this_ptr_;

        EntityConfigM_GetEntity_t fn_get_entity = nullptr;
    };
}
#endif //ISAACCOYOTE_ENTITY_CONFIG_MANAGER_H
