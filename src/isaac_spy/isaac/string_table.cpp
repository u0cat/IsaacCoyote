//
// Created by TsCat on 2026/7/11.
//

#include "isaac_spy/isaac/string_table.h"

#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"
#include "isaac_spy/scanner.h"

using namespace isaac_spy::isaac;
using namespace isaac_spy::constants;

StringTable::StringTable(uintptr_t ptr) : this_ptr_(ptr) {
    init();
}

int StringTable::get_language() {
    language_ = mem::read_value<int>(this_ptr_ + kOffsetStringTableLanguage);

    return language_;
}

std::string StringTable::get_string(const char* category, Language language, const char* key, bool& retBool) {
    if (!initialized) init();
    if (fn_get_string == nullptr) {
        retBool = false;
        return "";
    };

    bool game_flag = true;
    char* result = fn_get_string(this_ptr_, category, language, key, &game_flag);
    if (result == nullptr) {
        retBool = false;
        return "";
    };

    // retBool may not work
    retBool = true;
    auto string_result = std::string(result);
    if (string_result.starts_with("StringTable::")) retBool = false;


    return string_result;
}

void StringTable::init() {
    auto result = mem::Scanner(kPatternStringTableGetString).scan();
    if (result.found) fn_get_string = reinterpret_cast<StringTable_GetString_t>(result.address);

    initialized = true;
};
