//
// Created by TsCat on 2026/7/12.
//

#ifndef ISAACSPY_ITEM_POLL_H
#define ISAACSPY_ITEM_POLL_H
#include <cstdint>

namespace isaac_spy::isaac
{
    class Player;

    typedef int (__thiscall *ItemPool_GetPillEffect_t)(uintptr_t self, int pill_color, uintptr_t player);

    class ItemPool {
    public:
        ItemPool(uintptr_t ptr);

        ItemPool(const ItemPool&) = delete;
        ItemPool& operator=(const ItemPool&) = delete;
        ItemPool(ItemPool&&) = delete;
        ItemPool& operator=(ItemPool&&) = delete;

        int get_pill_effect(int pill_color, const Player& player);

        uintptr_t get_this_ptr() { return this_ptr_; }

    private:
        bool initialized = false;
        void init();

        uintptr_t this_ptr_;
        ItemPool_GetPillEffect_t fn_get_pill_effect = nullptr;
    };
}
#endif //ISAACSPY_ITEM_POLL_H
