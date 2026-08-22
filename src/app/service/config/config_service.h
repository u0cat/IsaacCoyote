//
// Created by TsCat on 2026/7/15.
//

#ifndef ISAACCOYOTE_CONFIG_H
#define ISAACCOYOTE_CONFIG_H
#include <filesystem>
#include <memory>

#include "config_struct.h"

namespace app::config
{
    class ConfigService {
    public:
        using Snapshot = std::shared_ptr<const AppConfig>;

        ConfigService(std::filesystem::path path);

        bool load();
        bool save();
        bool save(const AppConfig& config);
        void publish(AppConfig config);
        void reset();

        Snapshot snapshot() const;

    private:
        std::filesystem::path path_;
        Snapshot snapshot_;
    };
}
#endif //ISAACCOYOTE_CONFIG_H
