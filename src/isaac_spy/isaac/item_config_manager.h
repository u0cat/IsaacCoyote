//
// Created by TsCat on 2026/7/11.
//

#ifndef ISAACSPY_ITEM_CONFIG_MANAGER_H
#define ISAACSPY_ITEM_CONFIG_MANAGER_H

#include <string>
#include <unordered_map>

#include "enums.h"

namespace isaac_spy::isaac
{

    struct ItemConfig {
        int id = 0;
        enums::ItemType type = enums::ITEM_NULL;
        enums::CardType card_type = enums::CARDTYPE_NOT_CARD;
        int quality = 0;
        std::string name;
        std::string description;
    };

    class ItemConfigManager {
    public:
        explicit ItemConfigManager(uintptr_t ptr);
        ~ItemConfigManager() = default;

        ItemConfigManager(const ItemConfigManager&) = delete;
        ItemConfigManager& operator=(const ItemConfigManager&) = delete;

        ItemConfigManager(ItemConfigManager&&) noexcept = default;
        ItemConfigManager& operator=(ItemConfigManager&&) noexcept = default;

        const std::unordered_map<int, ItemConfig>& get_all_collectibles();
        const std::unordered_map<int, ItemConfig>& get_all_trinkets();
        const std::unordered_map<int, ItemConfig>& get_all_cards();
        const std::unordered_map<int, ItemConfig>& get_all_pills();

        const ItemConfig* get_collectible(int id);
        const ItemConfig* get_trinket(int id);
        const ItemConfig* get_card(int id);
        const ItemConfig* get_pill(int id);

    private:
        void load_all();

        uintptr_t this_ptr_;
        std::unordered_map<int, ItemConfig> collectibles_;
        std::unordered_map<int, ItemConfig> trinkets_;
        std::unordered_map<int, ItemConfig> cards_;
        std::unordered_map<int, ItemConfig> pills_;
        bool loaded_ = false;
    };
}
#endif
