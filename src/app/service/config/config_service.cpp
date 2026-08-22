#include "app/service/config/config_service.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "app/service/log/log_service.h"

namespace app::config
{
    namespace
    {
        spdlog::logger& log_()
        {
            static auto logger = app::log::get("app.config");
            return *logger;
        }
    }

    ConfigService::ConfigService(std::filesystem::path path)
        : path_(std::move(path)), snapshot_(std::make_shared<const AppConfig>(kDefaultConfig)) {}

    bool ConfigService::load() {
        std::ifstream file(path_);
        if (!file.is_open()) {
            log_().warn("failed to open config file: {}", path_.string());
            return false;
        }

        try {
            nlohmann::json j;
            file >> j;
            auto loaded = std::make_shared<const AppConfig>(j.get<AppConfig>());
            log_().info("config loaded: {} (version={})", path_.string(), loaded->version);
            snapshot_ = std::move(loaded);
            return true;
        }
        catch (const nlohmann::json::exception& e) {
            log_().error("failed to parse config {}: {}", path_.string(), e.what());
            return false;
        }
    }

    bool ConfigService::save() {
        return save(*snapshot());
    }

    bool ConfigService::save(const AppConfig& config) {
        try {
            auto dir = path_.parent_path();
            if (!dir.empty())
                std::filesystem::create_directories(dir);

            auto temp_path = path_;
            temp_path += ".tmp";
            std::ofstream file(temp_path, std::ios::trunc);
            if (!file.is_open()) {
                log_().warn("failed to open config for writing: {}", temp_path.string());
                return false;
            }

            nlohmann::json j = config;
            file << j.dump(4);
            file.close();
            if (!file) {
                log_().warn("failed to write config: {}", temp_path.string());
                return false;
            }

            std::error_code error;
            std::filesystem::remove(path_, error);
            error.clear();
            std::filesystem::rename(temp_path, path_, error);
            if (error) {
                log_().error("failed to replace config {}: {}", path_.string(), error.message());
                std::filesystem::remove(temp_path, error);
                return false;
            }
            log_().info("config saved: {}", path_.string());
            return true;
        }
        catch (const nlohmann::json::exception& e) {
            log_().error("failed to serialize config: {}", e.what());
            return false;
        }
    }

    void ConfigService::publish(AppConfig config) {
        auto published = std::make_shared<const AppConfig>(std::move(config));
        snapshot_ = std::move(published);
    }

    void ConfigService::reset() {
        auto reset = std::make_shared<const AppConfig>(kDefaultConfig);
        snapshot_ = std::move(reset);
    }

    ConfigService::Snapshot ConfigService::snapshot() const {
        return snapshot_;
    }
} // namespace config
