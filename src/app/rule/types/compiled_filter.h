//
// Created by TsCat on 2026/8/19.
//

#ifndef ISAACCOYOTE_COMPILED_FILTER_H
#define ISAACCOYOTE_COMPILED_FILTER_H

#include <unordered_set>
#include <vector>

// Filter shapes shared by the per-event descriptors (leaf: no rule-type dependencies)
namespace app::rule
{
    struct CompiledEntityKey {
        int type = -1;
        int subtype = -1;
        int variant = -1;
    };

    struct CompiledEntityFilter {
        bool whitelist = false;
        std::vector<CompiledEntityKey> whitelist_entities;
        std::vector<CompiledEntityKey> blacklist_entities;
    };

    struct CompiledItemFilter {
        bool whitelist = false;
        std::unordered_set<int> whitelist_items;
        std::unordered_set<int> blacklist_items;
    };
}

#endif //ISAACCOYOTE_COMPILED_FILTER_H
