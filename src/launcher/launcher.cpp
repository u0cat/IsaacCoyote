// Created by TsCat on 2026/7/30.

#include "launcher/launcher.h"

#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tlhelp32.h>
#include <vector>

namespace
{
    constexpr std::wstring_view kTargetProcess = L"isaac-ng.exe";
    constexpr std::wstring_view kDllName = L"IsaacCoyote.dll";
    constexpr DWORD kInjectionTimeoutMs = 30'000;

    struct HandleCloser
    {
        using pointer = void*;

        void operator()(const HANDLE handle) const noexcept
        {
            if (handle && handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
        }
    };

    using UniqueHandle = std::unique_ptr<void, HandleCloser>;

    std::wstring windows_error(const DWORD error = GetLastError())
    {
        wchar_t* message = nullptr;
        const DWORD length = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            0,
            reinterpret_cast<wchar_t*>(&message),
            0,
            nullptr
        );

        std::wstring result = length != 0 ? std::wstring(message, length) : L"Unknown error";
        if (message)
            LocalFree(message);

        while (!result.empty() && (result.back() == L'\r' || result.back() == L'\n'))
            result.pop_back();
        return result + L" (" + std::to_wstring(error) + L")";
    }

    std::filesystem::path executable_directory()
    {
        std::vector<wchar_t> buffer(MAX_PATH);
        for (;;)
        {
            const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (length == 0)
                throw std::runtime_error("GetModuleFileNameW failed");
            if (length < buffer.size() - 1)
                return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
            buffer.resize(buffer.size() * 2);
        }
    }

    std::optional<DWORD> find_process_id(const std::wstring_view process_name)
    {
        UniqueHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
        if (snapshot.get() == INVALID_HANDLE_VALUE)
            return std::nullopt;

        PROCESSENTRY32W entry{.dwSize = sizeof(entry)};
        if (!Process32FirstW(snapshot.get(), &entry))
            return std::nullopt;

        do
        {
            if (_wcsicmp(entry.szExeFile, process_name.data()) == 0)
                return entry.th32ProcessID;
        }
        while (Process32NextW(snapshot.get(), &entry));

        return std::nullopt;
    }

    UniqueHandle module_snapshot(const DWORD process_id)
    {
        for (int attempt = 0; attempt < 8; ++attempt)
        {
            UniqueHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id));
            if (snapshot.get() != INVALID_HANDLE_VALUE || GetLastError() != ERROR_BAD_LENGTH)
                return snapshot;
        }
        return UniqueHandle(INVALID_HANDLE_VALUE);
    }

    std::optional<uintptr_t> find_remote_module(
        const DWORD process_id,
        const std::wstring_view module_name
    ) {
        UniqueHandle snapshot = module_snapshot(process_id);
        if (snapshot.get() == INVALID_HANDLE_VALUE)
            return std::nullopt;

        MODULEENTRY32W entry{.dwSize = sizeof(entry)};
        if (!Module32FirstW(snapshot.get(), &entry))
            return std::nullopt;

        do
        {
            if (_wcsicmp(entry.szModule, module_name.data()) == 0)
                return reinterpret_cast<uintptr_t>(entry.modBaseAddr);
        }
        while (Module32NextW(snapshot.get(), &entry));

        return std::nullopt;
    }

    bool inject(const DWORD process_id, const std::filesystem::path& dll_path)
    {
        UniqueHandle process(OpenProcess(
            PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
            PROCESS_VM_WRITE | PROCESS_VM_READ,
            FALSE,
            process_id
        ));
        if (!process)
        {
            std::wcerr << L"Failed to open isaac-ng.exe: " << windows_error() << L'\n';
            return false;
        }

        const HMODULE local_kernel32 = GetModuleHandleW(L"kernel32.dll");
        const auto local_load_library = reinterpret_cast<uintptr_t>(
            GetProcAddress(local_kernel32, "LoadLibraryW")
        );
        const auto remote_kernel32 = find_remote_module(process_id, L"kernel32.dll");
        if (!local_kernel32 || local_load_library == 0 || !remote_kernel32)
        {
            std::wcerr << L"Failed to resolve LoadLibraryW in the target process.\n";
            return false;
        }

        const auto remote_load_library = reinterpret_cast<LPTHREAD_START_ROUTINE>(
            *remote_kernel32 + local_load_library - reinterpret_cast<uintptr_t>(local_kernel32)
        );
        const std::wstring path = dll_path.wstring();
        const SIZE_T path_size = (path.size() + 1) * sizeof(wchar_t);
        void* remote_path = VirtualAllocEx(
            process.get(), nullptr, path_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
        );
        if (!remote_path)
        {
            std::wcerr << L"Failed to allocate memory in isaac-ng.exe: " << windows_error() << L'\n';
            return false;
        }

        if (!WriteProcessMemory(process.get(), remote_path, path.c_str(), path_size, nullptr))
        {
            std::wcerr << L"Failed to write the DLL path: " << windows_error() << L'\n';
            VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
            return false;
        }

        UniqueHandle thread(CreateRemoteThread(
            process.get(), nullptr, 0, remote_load_library, remote_path, 0, nullptr
        ));
        if (!thread)
        {
            std::wcerr << L"Failed to start LoadLibraryW: " << windows_error() << L'\n';
            VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
            return false;
        }

        const DWORD wait_result = WaitForSingleObject(thread.get(), kInjectionTimeoutMs);
        if (wait_result != WAIT_OBJECT_0)
        {
            std::wcerr << (wait_result == WAIT_TIMEOUT
                ? L"Timed out while loading IsaacCoyote.dll.\n"
                : L"Failed while waiting for LoadLibraryW: " + windows_error() + L"\n");
            // The remote thread may still read the path after a timeout, so its allocation must remain valid.
            return false;
        }

        DWORD load_result = 0;
        const bool got_result = GetExitCodeThread(thread.get(), &load_result) != FALSE;
        VirtualFreeEx(process.get(), remote_path, 0, MEM_RELEASE);
        if (!got_result)
        {
            std::wcerr << L"Failed to read the LoadLibraryW result: " << windows_error() << L'\n';
            return false;
        }
        if (load_result == 0)
        {
            std::wcerr << L"IsaacCoyote.dll could not be loaded by isaac-ng.exe.\n";
            return false;
        }

        return true;
    }
}

int launcher::run()
{
    try
    {
        const std::filesystem::path dll_path = executable_directory() / kDllName;
        if (!std::filesystem::is_regular_file(dll_path))
        {
            std::wcerr << L"IsaacCoyote.dll was not found next to the launcher:\n"
                       << dll_path.wstring() << L'\n';
            return 1;
        }

        const auto process_id = find_process_id(kTargetProcess);
        if (!process_id)
        {
            std::wcerr << L"isaac-ng.exe is not running. Start the game before using the launcher.\n";
            return 2;
        }

        if (find_remote_module(*process_id, kDllName))
        {
            std::wcout << L"IsaacCoyote.dll is already loaded.\n";
            return 0;
        }

        if (!inject(*process_id, dll_path))
            return 3;

        std::wcout << L"IsaacCoyote.dll was injected successfully.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Launcher error: " << error.what() << '\n';
        return 4;
    }
}

int main()
{
    return launcher::run();
}

