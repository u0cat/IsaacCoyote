//
// Created by TsCat on 2026/7/12.
//

#ifndef ISAACSPY_TRINKET_H
#define ISAACSPY_TRINKET_H
#include <string>
#include <unordered_map>

#include "string_table.h"

namespace isaac_spy::isaac { struct ItemConfig; }

namespace isaac_spy::isaac
{
    class TrinketDesc {
    public:
        TrinketDesc(int id);

        std::string get_localized_name(Language language, bool& retBool) const;
        const ItemConfig* get_config() const;

        int get_id() { return id_; }

    private:
        int id_;

        mutable std::unordered_map<Language, std::string> cached_names_;
        mutable const ItemConfig* config_ = nullptr;
    };
}
#endif //ISAACSPY_TRINKET_H
