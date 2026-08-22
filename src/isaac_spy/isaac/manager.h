//
// Created by TsCat on 2026/7/11.
//

#ifndef ISAACSPY_MANAGER_H
#define ISAACSPY_MANAGER_H
#include <memory>

#include "entity_config_manager.h"
#include "item_config_manager.h"
#include "netplay_manager.h"
#include "string_table.h"

namespace isaac_spy::isaac
{
    class Manager {
    public:
        static Manager& get_instance() {
            static Manager instance;
            if (!instance.is_initialized) instance.init();
            return instance;
        };

        ItemConfigManager* get_item_config_manager();
        EntityConfigManager* get_entity_config_manager();
        StringTable* get_string_table();
        NetPlayManager* get_netplay_manager();

        bool is_in_game() const;
        bool can_continue() const;
        bool can_rerun() const;
        uintptr_t get_this_ptr() const;

    private:
        Manager() = default;

        std::unique_ptr<NetPlayManager> netplay_manager_ = nullptr;
        std::unique_ptr<ItemConfigManager> item_config_manager_ = nullptr;
        std::unique_ptr<EntityConfigManager> entity_config_manager_ = nullptr;
        std::unique_ptr<StringTable> string_table_ = nullptr;

        void init();
        bool is_initialized = false;
        uintptr_t this_ptr_ = 0;
    };

    // restart 558bec6aff68????????64a1????????5083ec60a1????????33c58945??535657508d45??64a3????????c745
}
#endif //ISAACSPY_MANAGER_H
