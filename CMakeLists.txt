cmake_minimum_required(VERSION 4.2)

project(IsaacCoyote LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/utf-8)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
            "IsaacCoyote must be configured for x86. With a Visual Studio generator, "
            "pass -A Win32 and use the x86-windows-static vcpkg triplet."
    )
endif ()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(imgui CONFIG REQUIRED)
find_package(ixwebsocket CONFIG REQUIRED)
find_package(safetyhook CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# QR Code Generator (fetched from GitHub)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
        qrcodegen
        GIT_REPOSITORY https://github.com/nayuki/QR-Code-generator.git
        GIT_TAG master
)
FetchContent_MakeAvailable(qrcodegen)

add_library(qrcodegen STATIC ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.cpp)
target_include_directories(qrcodegen PUBLIC ${qrcodegen_SOURCE_DIR}/cpp)

# ---------------------------------------------------------------------------
# Embedded config (isaac-coyote.json -> generated/embedded_config.h)
# ---------------------------------------------------------------------------
set(EMBEDDED_CONFIG_JSON "${CMAKE_SOURCE_DIR}/isaac-coyote.json")
set(EMBEDDED_CONFIG_HEADER "${CMAKE_BINARY_DIR}/generated/embedded_config.h")
set(EMBEDDED_CONFIG_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/GenerateEmbeddedConfig.cmake")

# Immediate generation at configure time so the header exists for IDE / first build.
execute_process(
        COMMAND ${CMAKE_COMMAND}
        -DINPUT_JSON=${EMBEDDED_CONFIG_JSON}
        -DOUTPUT_HEADER=${EMBEDDED_CONFIG_HEADER}
        -P ${EMBEDDED_CONFIG_SCRIPT}
        RESULT_VARIABLE _embedded_gen_result
)
if(NOT _embedded_gen_result EQUAL 0)
    message(WARNING "Failed to generate embedded_config.h at configure time (result=${_embedded_gen_result})")
endif()

# Incremental regeneration when the JSON changes (Ninja/Make).
add_custom_command(
        OUTPUT ${EMBEDDED_CONFIG_HEADER}
        COMMAND ${CMAKE_COMMAND}
        -DINPUT_JSON=${EMBEDDED_CONFIG_JSON}
        -DOUTPUT_HEADER=${EMBEDDED_CONFIG_HEADER}
        -P ${EMBEDDED_CONFIG_SCRIPT}
        DEPENDS ${EMBEDDED_CONFIG_JSON} ${EMBEDDED_CONFIG_SCRIPT}
        COMMENT "Generating embedded_config.h from isaac-coyote.json"
        VERBATIM
)
add_custom_target(embedded_config_header DEPENDS ${EMBEDDED_CONFIG_HEADER})
# Expose the generated header as a source so IDEs see the dependency.
set_source_files_properties(${EMBEDDED_CONFIG_HEADER} PROPERTIES GENERATED TRUE)

# ---------------------------------------------------------------------------
# IsaacCoyote (x86 DLL)
# ---------------------------------------------------------------------------

# Entry point
set(ISAAC_COYOTE_ENTRY
        main.cpp
)

# Application layer: orchestration, rules, events, coyote service
set(ISAAC_COYOTE_APP
        # Core
        src/app/isaac_coyote.cpp
        src/app/game.cpp
        # Config
        src/app/service/config/config_service.cpp
        src/app/service/config/config_struct.cpp
        src/app/service/config/player_filter.cpp
        # Logging
        src/app/service/log/log_service.cpp
        # Events
        src/app/event/catalog.cpp
        src/app/event/event_engine.cpp
        src/app/event/event_source_manager.cpp
        src/app/event/descriptors/hurt.cpp
        src/app/event/descriptors/death.cpp
        src/app/event/descriptors/reroll.cpp
        src/app/event/descriptors/game_over.cpp
        src/app/event/descriptors/use_active_item.cpp
        src/app/event/descriptors/use_pill.cpp
        src/app/event/descriptors/use_card.cpp
        src/app/event/sources/hurt_source.cpp
        src/app/event/sources/reroll.cpp
        src/app/event/sources/player_death.cpp
        src/app/event/sources/game_over.cpp
        src/app/event/sources/use_active_item_source.cpp
        src/app/event/sources/use_pill_source.cpp
        src/app/event/sources/use_card_source.cpp
        # Rules
        src/app/rule/rule_engine.cpp
        src/app/rule/rule_handler_manager.cpp
        src/app/rule/rule_utils.cpp
        src/app/rule/state_hub.cpp
        src/app/rule/strength_resolver.cpp
        # Handlers - event
        src/app/rule/handlers/event/hurt_handler.cpp
        src/app/rule/handlers/event/death_handler.cpp
        src/app/rule/handlers/event/game_over_handler.cpp
        src/app/rule/handlers/event/reroll_handler.cpp
        src/app/rule/handlers/event/use_active_item_handler.cpp
        src/app/rule/handlers/event/use_pill_handler.cpp
        src/app/rule/handlers/event/use_card_handler.cpp
#        src/app/rule/handlers/event/collectible_changed_handler.cpp
#        src/app/rule/handlers/event/health_changed_handler.cpp
        # Handlers - static
        src/app/rule/handlers/static/health_source_handler.cpp
        src/app/rule/handlers/static/collectible_source_handler.cpp
        # Coyote service (WebSocket + pulse helper)
        src/app/service/coyote/coyote_service.cpp
        src/app/service/coyote/websocket/controller.cpp
        src/app/service/coyote/websocket/models.cpp
        src/app/service/coyote/websocket/client.cpp
        src/app/service/coyote/pulse_helper/pulse_helper.cpp
        src/app/service/coyote/pulse_helper/types.cpp
)

# Overlay: GDI hook and ImGui UI
set(ISAAC_COYOTE_OVERLAY
        src/app/service/overlay/coyote_monitor.cpp
        src/app/service/overlay/core/imgui_renderer.cpp
        src/app/service/overlay/core/input_state.cpp
        src/app/service/overlay/core/hook.cpp
        src/app/service/overlay/overlay_ui.cpp
        src/app/service/overlay/overlay_service.cpp
        src/app/service/overlay/tab_manager.cpp
        src/app/service/overlay/tabs/config/config_tab.cpp
        src/app/service/overlay/tabs/config/pages/connection.cpp
        src/app/service/overlay/tabs/config/pages/game.cpp
        src/app/service/overlay/tabs/config/pages/static_rules.cpp
        src/app/service/overlay/tabs/config/pages/events.cpp
        src/app/service/overlay/tabs/config/pages/pickers.cpp
        src/app/service/overlay/tabs/config/pages/logging.cpp
        src/app/service/overlay/tabs/debug_tab.cpp
        src/app/service/overlay/tabs/connection_tab.cpp
        src/app/service/overlay/tabs/statistics_tab.cpp
        src/app/service/overlay/ui/themes.cpp
        src/app/service/overlay/ui/themes.h
        src/app/service/overlay/ui/components.cpp
        src/app/service/overlay/ui/components.h
)

# Isaac Spy: memory scanning, hooks, game object wrappers
set(ISAAC_COYOTE_ISAAC_SPY
        # Core
        src/isaac_spy/hook_manager.cpp
        src/isaac_spy/scanner.cpp
        src/isaac_spy/watch_manager.cpp
        # Hooks
        src/isaac_spy/hooks/hurt_hook.cpp
        src/isaac_spy/hooks/use_active_item_hook.cpp
        src/isaac_spy/hooks/use_pill_hook.cpp
        src/isaac_spy/hooks/use_card_hook.cpp
        # Game objects
        src/isaac_spy/isaac/collectible.cpp
        src/isaac_spy/isaac/entity.cpp
        src/isaac_spy/isaac/game.cpp
        src/isaac_spy/isaac/item_config_manager.cpp
        src/isaac_spy/isaac/item_pool.cpp
        src/isaac_spy/isaac/manager.cpp
        src/isaac_spy/isaac/netplay_manager.cpp
        src/isaac_spy/isaac/player.cpp
        src/isaac_spy/isaac/player_manager.cpp
        src/isaac_spy/isaac/pocket_item.cpp
        src/isaac_spy/isaac/room.cpp
        src/isaac_spy/isaac/string_table.cpp
        src/isaac_spy/isaac/trinket.cpp
        src/isaac_spy/hooks/restart_game_hook.cpp
        src/isaac_spy/hooks/start_new_game_hook.cpp
        src/isaac_spy/hooks/player_death_hook.cpp
        src/isaac_spy/isaac/entity_ref.cpp
        src/isaac_spy/isaac/entity_config_manager.cpp
)

# Utilities
set(ISAAC_COYOTE_UTILS
        src/utils/utils.cpp
)

add_library(IsaacCoyote SHARED
        ${ISAAC_COYOTE_ENTRY}
        ${ISAAC_COYOTE_APP}
        ${ISAAC_COYOTE_OVERLAY}
        ${ISAAC_COYOTE_ISAAC_SPY}
        ${ISAAC_COYOTE_UTILS}
)

target_include_directories(IsaacCoyote PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_BINARY_DIR}/generated"
)

target_compile_definitions(IsaacCoyote PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
)

# Ensure the embedded header is generated before any compilation unit that may include it.
add_dependencies(IsaacCoyote embedded_config_header)

target_link_libraries(IsaacCoyote PRIVATE
        imgui::imgui
        ixwebsocket::ixwebsocket
        nlohmann_json::nlohmann_json
        safetyhook::safetyhook
        spdlog::spdlog
        OpenGL::GL
        qrcodegen
        bcrypt
)

# ---------------------------------------------------------------------------
# IsaacCoyote Launcher (x86 executable)
# ---------------------------------------------------------------------------
add_executable(IsaacCoyoteLauncher
        src/launcher/launcher.cpp
        src/launcher/launcher.h
)

target_include_directories(IsaacCoyoteLauncher PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(IsaacCoyoteLauncher PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
)

add_dependencies(IsaacCoyoteLauncher IsaacCoyote)

# ---------------------------------------------------------------------------
# Compile database deployment
# ---------------------------------------------------------------------------
add_custom_target(deploy_compile_commands ALL
        ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${CMAKE_SOURCE_DIR}/compile_commands.json"
        COMMENT "Deploying compile_commands.json to project root"
)
