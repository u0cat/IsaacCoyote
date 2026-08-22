//
// Created by TsCat on 2026/7/9.
//
#include "isaac_spy/scanner.h"

#include <windows.h>

using namespace isaac_spy::mem;

unsigned char* Scanner::s_module_base = 0;
size_t Scanner::s_base_size = 0;
unsigned char* Scanner::s_last_address = 0;

Scanner::Scanner(std::string_view pattern) {
    if (s_module_base == 0) init();
    valid_ = !pattern.empty() && parse_pattern(pattern);
}

bool Scanner::valid() const {
    return valid_;
}

void Scanner::init() {
    s_module_base = (unsigned char*)GetModuleHandle(NULL);

    IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)s_module_base;
    IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(s_module_base + dos_header->e_lfanew);

    s_base_size = nt_header->OptionalHeader.SizeOfImage;

    s_last_address = s_module_base;
}

bool Scanner::parse_pattern(std::string_view pattern) {
    auto pattern_size = pattern.size();
    sign_bytes.reserve(pattern_size / 2);
    sign_mask.reserve(pattern_size / 2);

    size_t i = 0;
    int byte_index = 0;
    int capture_start = -1;

    const auto hex_to_nibble = [](char c) -> int
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    while (i < pattern_size) {
        char c = pattern[i];

        if (c == '(') {
            if (capture_start != -1) return false; // 不支持嵌套括号
            capture_start = byte_index;
            ++i;
            continue;
        }

        if (c == ')') {
            if (capture_start == -1) return false; // 括号不成对
            Match match = {nullptr, capture_start, byte_index - capture_start};
            captures.push_back(match);
            capture_start = -1;
            ++i;
            continue;
        }

        if (i + 1 >= pattern_size) return false; // 不完整的十六进制对

        char c1 = pattern[i];
        char c2 = pattern[i + 1];
        int high = hex_to_nibble(c1);
        int low = hex_to_nibble(c2);

        if (high >= 0 && low >= 0) {
            sign_bytes.push_back(static_cast<uint8_t>((high << 4) | low));
            sign_mask.push_back(1);
        }
        else if (c1 == '?' && c2 == '?') {
            sign_bytes.push_back(0);
            sign_mask.push_back(0);
        }
        else {
            return false;
        }

        ++byte_index;
        i += 2;
    }

    if (capture_start != -1) return false;
    sign_bytes_size = sign_bytes.size();
    return sign_bytes_size != 0;
}

Scanner::ScanResult Scanner::scan(bool allow_multiple, bool start_from_last) {
    ScanResult result;
    if (!valid_ || sign_bytes_size == 0) {
        return result;
    }

    unsigned char* pos_start = s_module_base;
    if (start_from_last) pos_start = s_last_address;

    unsigned char* pos_end = s_module_base + s_base_size - sign_bytes_size;

    auto match_pattern = [&](const unsigned char* addr) -> bool
    {
        for (size_t i = 0; i < sign_bytes_size; ++i) {
            if (sign_mask[i] && addr[i] != sign_bytes[i])
                return false;
        }
        return true;
    };

    bool found_any = false;
    for (; pos_start <= pos_end; ++pos_start) {
        if (!match_pattern(pos_start))
            continue;

        // success
        if (start_from_last)
            result.distance = pos_start - s_last_address;
        else
            result.distance = pos_start - s_module_base;

        auto* const matched_addr = pos_start;

        s_last_address = pos_start + sign_bytes_size;

        for (auto& match : captures) {
            unsigned char* src = matched_addr + match.begin;
            uintptr_t v = 0;
            for (int i = match.length - 1; i >= 0; --i)
                v = (v << 8) | src[i];
            match.value = reinterpret_cast<unsigned char*>(v);
            result.captures.push_back(match);
        }

        if (allow_multiple) {
            result.addresses.push_back(matched_addr);
            found_any = true;
        }
        else {
            result.address = matched_addr;
            result.found = true;
            return result;
        }
    }

    if (allow_multiple) {
        result.found = found_any;
    }
    else {
        result.address = nullptr;
    }

    // 未找到时重置 s_last_address
    if (!found_any && start_from_last) {
        s_last_address = s_module_base;
    }

    return result;
}
