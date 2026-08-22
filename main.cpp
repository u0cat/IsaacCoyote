#include <windows.h>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <stdexcept>

#include "app/isaac_coyote.h"
#include "app/service/log/log_service.h"

namespace {
    bool g_console_opened = true;
}

void console_toggle()
{
    g_console_opened = !g_console_opened;
    ShowWindow(GetConsoleWindow(), g_console_opened ? SW_SHOW : SW_HIDE);
}

HMODULE g_module;

DWORD WINAPI main_thread(LPVOID)
{
    AllocConsole();
    console_toggle();
    freopen("CONOUT$", "w", stdout);
    SetConsoleTitleA("IsaacCoyote Console");

    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hConsole, &mode))
    {
        mode |= ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS;
        SetConsoleMode(hConsole, mode);
    }

    std::filesystem::path dll_dir;
    wchar_t module_path[MAX_PATH];
    const DWORD path_len = GetModuleFileNameW(g_module, module_path, MAX_PATH);
    if (path_len > 0 && path_len < MAX_PATH)
        dll_dir = std::filesystem::path(module_path).parent_path();

    app::log::init(dll_dir);
    app::log::get("app.main")->info("DLL loaded");
    if (!dll_dir.empty())
        app::log::get("app.main")->info("log file: {}", (dll_dir / "isaac-coyote.log").string());

    try
    {
        app::IsaacCoyote::get_instance().run();

        while (!(GetAsyncKeyState(VK_END) & 1))
            Sleep(50);

        app::IsaacCoyote::get_instance().stop();
        Sleep(200);
    }
    catch (const std::exception& error)
    {
        app::log::get("app.main")->critical("Fatal error: {}", error.what());
        app::IsaacCoyote::get_instance().stop();
        Sleep(1000);
    }
    catch (...)
    {
        app::log::get("app.main")->critical("Fatal unknown error");
        app::IsaacCoyote::get_instance().stop();
        Sleep(1000);
    }

    app::log::get("app.main")->info("IsaacCoyote unloaded");
    app::log::shutdown();
    FreeConsole();
    FreeLibraryAndExitThread(g_module, 0);
    // return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    g_module = module;

    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);

        HANDLE thread = CreateThread(nullptr, 0, main_thread, nullptr, 0, nullptr);
        if (thread)
            CloseHandle(thread);
    }

    return TRUE;
}
