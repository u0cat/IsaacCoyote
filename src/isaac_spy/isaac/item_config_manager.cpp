#include "isaac_spy/isaac/item_config_manager.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

namespace
{
    using namespace isaac_spy::mem;

    template <typename F>
    void read_into(uintptr_t base, uintptr_t begin_off, uintptr_t end_off,
                   std::unordered_map<int, ItemConfig>& out, uintptr_t id_off, F&& make) {
        uintptr_t begin = read_ptr(base + begin_off);
        uintptr_t end = read_ptr(base + end_off);

        if (begin == 0 || end <= begin)
            return;

        out.reserve((end - begin) / 4);

        for (uintptr_t addr = begin; addr < end; addr += 4) {
            if (uintptr_t item = read_ptr(addr); item != 0)
                out.emplace(read_value<int>(item + id_off), make(item));
        }
    }

    ItemConfig make_collectible_or_trinket(uintptr_t item) {
        return ItemConfig{
            read_value<int>(item + kOffsetItemConfigItemId),
            static_cast<enums::ItemType>(read_value<int>(item + kOffsetItemConfigType)),
            enums::CARDTYPE_NOT_CARD,
            read_value<int>(item + kOffsetItemConfigQuality),
            read_string(item + kOffsetItemConfigName),
            read_string(item + kOffsetItemConfigDesc),
        };
    }

    ItemConfig make_card(uintptr_t item) {
        return ItemConfig{
            read_value<int>(item + kOffsetItemConfigCardPillId),
            enums::ITEM_NOT_ITEM,
            static_cast<enums::CardType>(read_value<int>(item + kOffsetItemConfigCardType)),
            -1,
            read_string(item + kOffsetItemConfigName),
            read_string(item + kOffsetItemConfigDesc),
        };
    }

    ItemConfig make_pill(uintptr_t item) {
        return ItemConfig{
            read_value<int>(item + kOffsetItemConfigCardPillId),
            enums::ITEM_NOT_ITEM,
            enums::CARDTYPE_NOT_CARD,
            -1,
            read_string(item + kOffsetItemConfigName),
            read_string(item + kOffsetItemConfigDesc),
        };
    }
}

ItemConfigManager::ItemConfigManager(uintptr_t ptr) : this_ptr_(ptr) {}

void ItemConfigManager::load_all() {
    if (loaded_)
        return;

    read_into(this_ptr_, kOffsetItemTableCollectiblesBegin, kOffsetItemTableCollectiblesEnd, collectibles_, kOffsetItemConfigItemId, make_collectible_or_trinket);
    read_into(this_ptr_, kOffsetItemTableTrinketsBegin, kOffsetItemTableTrinketsEnd, trinkets_, kOffsetItemConfigItemId, make_collectible_or_trinket);
    read_into(this_ptr_, kOffsetItemTableCardsBegin, kOffsetItemTableCardsEnd, cards_, kOffsetItemConfigCardPillId, make_card);
    read_into(this_ptr_, kOffsetItemTablePillsBegin, kOffsetItemTablePillsEnd, pills_, kOffsetItemConfigCardPillId, make_pill);

    loaded_ = true;
}

const std::unordered_map<int, ItemConfig>& ItemConfigManager::get_all_collectibles() {
    load_all();
    return collectibles_;
}

const std::unordered_map<int, ItemConfig>& ItemConfigManager::get_all_trinkets() {
    load_all();
    return trinkets_;
}

const std::unordered_map<int, ItemConfig>& ItemConfigManager::get_all_cards() {
    load_all();
    return cards_;
}

const std::unordered_map<int, ItemConfig>& ItemConfigManager::get_all_pills() {
    load_all();
    return pills_;
}

const ItemConfig* ItemConfigManager::get_collectible(int id) {
    load_all();

    auto it = collectibles_.find(id);
    return (it != collectibles_.end()) ? &it->second : nullptr;
}

const ItemConfig* ItemConfigManager::get_trinket(int id) {
    load_all();
    auto it = trinkets_.find(id);
    return (it != trinkets_.end()) ? &it->second : nullptr;
}

const ItemConfig* ItemConfigManager::get_card(int id) {
    load_all();
    auto it = cards_.find(id);
    return (it != cards_.end()) ? &it->second : nullptr;
}

const ItemConfig* ItemConfigManager::get_pill(int id) {
    load_all();
    auto it = pills_.find(id);
    return (it != pills_.end()) ? &it->second : nullptr;
}
