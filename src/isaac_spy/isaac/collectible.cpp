//
// Created by TsCat on 2026/7/11.
//

#include "isaac_spy/isaac/collectible.h"

#include <cinttypes>

#include "isaac_spy/isaac/manager.h"

using namespace isaac_spy::isaac;

CollectibleDesc::CollectibleDesc(int id) : id_(id) {
    config_ = nullptr;
}

std::string CollectibleDesc::get_localized_name(Language language, bool& retBool) const {
    auto it = cached_names_.find(language);
    if (it != cached_names_.end()) {
        retBool = true;
        return it->second;
    }

    if (!config_) {
        config_ = get_config();
    }
    if (!config_) {
        retBool = false;
        return {};
    }

    std::string_view key = config_->name;
    if (!key.empty() && key[0] == '#') {
        key.remove_prefix(1);
    }
    else {
        retBool = true;
        return config_->name;
    }

    auto* string_table = Manager::get_instance().get_string_table();
    std::string name = string_table->get_string("Items", language, key.data(), retBool);
    if (retBool) {
        cached_names_[language] = name;
        return name;
    }
    return {};
}

const ItemConfig* CollectibleDesc::get_config() const {
    if (config_ != nullptr) return config_;

    if (auto* item_config_manager = Manager::get_instance().get_item_config_manager()) {
        config_ = item_config_manager->get_collectible(id_);
    }
    return config_;
}
