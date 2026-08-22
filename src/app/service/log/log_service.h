//
// Created by TsCat on 2026/8/18.
//

#ifndef ISAACCOYOTE_LOG_SERVICE_H
#define ISAACCOYOTE_LOG_SERVICE_H

#include <filesystem>
#include <memory>
#include <string>

#include <spdlog/logger.h>

#include "app/service/config/config_struct.h"

namespace app::log
{
    // Creates all named loggers (file at <log_dir>/isaac-coyote.log, truncated)
    // plus a colored console sink. Safe to call once; later calls are no-ops.
    void init(const std::filesystem::path& log_dir);

    // Returns the named logger, creating it on demand (default level) if absent.
    std::shared_ptr<spdlog::logger> get(const std::string& name);

    // Applies the global level to every logger, then per-logger overrides.
    // Invalid level strings fall back with a warning on app.main.
    void apply_config(const config::LoggingConfig& config);

    // Flushes and drops all loggers. Call once before console teardown.
    void shutdown();
}

#endif //ISAACCOYOTE_LOG_SERVICE_H