//
// Created by TsCat on 2026/8/18.
//

#include "app/service/log/log_service.h"

#include <atomic>
#include <cstdio>
#include <exception>
#include <mutex>
#include <vector>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace app::log
{
    namespace
    {
        constexpr std::string_view kLogFileName = "isaac-coyote.log";
        constexpr std::string_view kPattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v";

        constexpr std::string_view kLoggerNames[] = {
            "app.main", "app.config", "app.game", "app.event", "app.rule",
            "app.coyote", "app.overlay", "isaac_spy",
        };

        std::once_flag g_init_flag;
        std::vector<std::shared_ptr<spdlog::sinks::sink>> g_default_sinks;
        std::atomic<spdlog::level::level_enum> g_default_level{spdlog::level::info};

        std::optional<spdlog::level::level_enum> parse_level(const std::string& name)
        {
            if (name == "trace") return spdlog::level::trace;
            if (name == "debug") return spdlog::level::debug;
            if (name == "info") return spdlog::level::info;
            if (name == "warn") return spdlog::level::warn;
            if (name == "error") return spdlog::level::err;
            if (name == "critical") return spdlog::level::critical;
            if (name == "off") return spdlog::level::off;
            return std::nullopt;
        }

        void warn_on_main(const std::string& message)
        {
            if (auto main = spdlog::get("app.main")) main->warn("{}", message);
        }

        std::shared_ptr<spdlog::logger> create_logger(const std::string& name, spdlog::level::level_enum level)
        {
            auto logger = std::make_shared<spdlog::logger>(name, g_default_sinks.begin(), g_default_sinks.end());
            logger->set_level(level);
            logger->set_pattern(std::string{kPattern});
            logger->flush_on(spdlog::level::err);
            spdlog::register_logger(logger);
            return logger;
        }
    }

    void init(const std::filesystem::path& log_dir)
    {
        std::call_once(g_init_flag, [&]
        {
            try
            {
                if (!log_dir.empty())
                    std::filesystem::create_directories(log_dir);
                g_default_sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                    (log_dir / kLogFileName).string(), true));
            }
            catch (const std::exception& e)
            {
                // Loggers are not registered yet inside init, so fall back to the console directly.
                std::fprintf(stderr, "[app.main] [warning] file sink unavailable, console only: %s\n", e.what());
            }

            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_pattern(std::string{kPattern});
            g_default_sinks.push_back(std::move(console_sink));

            for (const auto& name : kLoggerNames)
            {
                create_logger(std::string{name}, g_default_level.load());
            }
        });
    }

    std::shared_ptr<spdlog::logger> get(const std::string& name)
    {
        if (auto existing = spdlog::get(name)) return existing;
        return create_logger(name, g_default_level.load());
    }

    void apply_config(const config::LoggingConfig& config)
    {
        const auto global_level = parse_level(config.level);
        if (!global_level)
            warn_on_main("invalid log level \"" + config.level + "\", using info");
        const auto level = global_level.value_or(spdlog::level::info);

        g_default_level.store(level, std::memory_order_relaxed);
        spdlog::apply_all([level](std::shared_ptr<spdlog::logger> logger)
        {
            logger->set_level(level);
        });

        for (const auto& [name, level_name] : config.loggers)
        {
            const auto override_level = parse_level(level_name);
            if (!override_level)
            {
                warn_on_main("invalid log level \"" + level_name + "\" for logger \"" + name + "\", keeping global");
                continue;
            }
            auto logger = spdlog::get(name);
            if (!logger) logger = create_logger(name, level);
            logger->set_level(*override_level);
        }
    }

    void shutdown()
    {
        spdlog::apply_all([](std::shared_ptr<spdlog::logger> logger) { logger->flush(); });
        spdlog::shutdown();
    }
} // namespace app::log