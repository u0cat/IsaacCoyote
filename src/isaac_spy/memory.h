//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_MEMORY_H
#define ISAACSPY_MEMORY_H
#include <windows.h>
#include <excpt.h>
#include <cstdint>
#include <cstring>
#include <string>

namespace isaac_spy::mem
{
    inline bool safe_read_raw(void* dest, const uintptr_t src, size_t size) {
        if (!dest || !src) {
            return false;
        }

        SIZE_T bytes_read = 0;
        return ReadProcessMemory(
                GetCurrentProcess(),
                (LPCVOID)src,
                dest,
                size,
                &bytes_read) != FALSE &&
            bytes_read == size;
    }


    inline uintptr_t read_ptr(uintptr_t address) {
        if (!address) {
            return 0;
        }

        uintptr_t value = 0;
        if (!safe_read_raw(
            &value,
            address,
            sizeof(value))) {
            return 0;
        }

        return value;
    }

    template <typename T>
    T read_value(uintptr_t address) {
        T value{};
        safe_read_raw(&value, address, sizeof(T));
        return value;
    }

    inline std::string read_string(uintptr_t string_address) {
        uint32_t length = 0;
        if (!safe_read_raw(&length, string_address + 0x10, sizeof(length)))
            return "";
        if (length > 4096) return "";

        uint32_t capacity = 0;
        if (!safe_read_raw(&capacity, string_address + 0x14, sizeof(capacity)))
            return "";
        if (length > capacity) return "";

        uintptr_t data_addr = string_address;
        if (capacity >= 16) {
            data_addr = read_ptr(string_address);
            if (!data_addr && length > 0) return "";
        }

        if (length == 0) return "";

        std::string result(length, '\0');
        if (!safe_read_raw(result.data(), data_addr, length))
            return "";

        return result;
    }


    template <typename T>
    bool write(uintptr_t address, const T& value) {
        if (!address) return false;

        const SIZE_T size = sizeof(T);
        auto* dest = reinterpret_cast<unsigned char*>(address);

        // Protect
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(dest, &mbi, sizeof(mbi))) return false;
        const bool needProtect = !(mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY |
            PAGE_EXECUTE_WRITECOPY));
        DWORD old_protect = 0;
        bool prot_changed = false;

        if (needProtect) {
            if (!VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &old_protect))
                return false;
            prot_changed = true;
        }

        bool write_succeeded = false;
        __try {
            std::memcpy(dest, &value, size);
            write_succeeded = true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            write_succeeded = false;
        }

        if (prot_changed) {
            DWORD unused_protect = 0;
            if (!VirtualProtect(dest, size, old_protect, &unused_protect)) {
                return false;
            }
        }

        return write_succeeded;
    }
}
#endif //ISAACSPY_MEMORY_H
