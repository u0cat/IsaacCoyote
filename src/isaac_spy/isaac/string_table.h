//
// Created by TsCat on 2026/7/11.
//

#ifndef ISAACSPY_STRING_TABLE_H
#define ISAACSPY_STRING_TABLE_H
#include <string>

namespace isaac_spy::isaac
{
    typedef char* (__thiscall *StringTable_GetString_t)(uintptr_t self, const char* category, int language,
                                                        const char* key,
                                                        bool* retBool);

    enum Language {
        LANGUAGE_ENGLISH = 0,
        LANGUAGE_JAPANESE = 2,
        LANGUAGE_FRENCH = 3,
        LANGUAGE_SPANISH = 4,
        LANGUAGE_GERMAN = 5,
        LANGUAGE_RUSSIAN = 10,
        LANGUAGE_KOREAN = 11,
        LANGUAGE_CHINESE = 13,
    };

    class StringTable {
    public:
        StringTable(uintptr_t ptr);

        int get_language();
        std::string get_string(const char* category, Language language, const char* key, bool& retBool);

    private:
        bool initialized = false;
        uintptr_t this_ptr_;

        int language_ = 1; //1 -> english
        StringTable_GetString_t fn_get_string = nullptr;

        void init();
    };
}
#endif //ISAACSPY_STRING_TABLE_H
