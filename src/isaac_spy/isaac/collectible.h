//
// Created by TsCat on 2026/7/11.
//

#ifndef ISAACSPY_COLLECTIBLE_H
#define ISAACSPY_COLLECTIBLE_H
#include <string>
#include <unordered_map>

#include "string_table.h"

namespace isaac_spy::isaac { struct ItemConfig; }

namespace isaac_spy::isaac
{
    class CollectibleDesc {
    public:
        CollectibleDesc(int id);

        std::string get_localized_name(Language language, bool& retBool) const;
        const ItemConfig* get_config() const;

        int get_id() { return id_; }

    private:
        int id_;

        mutable std::unordered_map<Language, std::string> cached_names_;
        mutable const ItemConfig* config_;
    };
}
#endif //ISAACSPY_COLLECTIBLE_H
