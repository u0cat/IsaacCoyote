//
// Created by TsCat on 2026/7/11.
//

#ifndef ISAACSPY_POCKET_ITEM_H
#define ISAACSPY_POCKET_ITEM_H
#include <string>
#include <unordered_map>

#include "string_table.h"

namespace isaac_spy::isaac { struct ItemConfig; }

namespace isaac_spy::isaac
{
    class PocketItemDesc {
    public:
        PocketItemDesc(int id, int type);

        std::string get_localized_name(Language language, bool& retBool) const;
        const ItemConfig* get_config() const;

        int get_id() { return id_; }
        int get_type() { return type_; }
        bool is_pill() { return type_ == 0; }
        bool is_card() { return type_ == 1; }
        bool is_item() { return type_ == 2; }

    private:
        int id_;
        int type_;
        // pill = 0
        // card & rune = 1
        // item = 2

        mutable std::unordered_map<Language, std::string> cached_names_;
        mutable const ItemConfig* config_ = nullptr;
    };
}
#endif //ISAACSPY_POCKET_ITEM_H
