//
// Created by TsCat on 2026/7/9.
//

#ifndef ISAACSPY_SCANNER_H
#define ISAACSPY_SCANNER_H
#include <string_view>
#include <vector>

namespace isaac_spy::mem
{
    class Scanner {
    public:
        struct Match {
            unsigned char* value;
            int begin;
            int length;
        };

        struct ScanResult {
            bool found = false;
            unsigned char* address = nullptr;
            std::vector<unsigned char*> addresses;
            std::vector<Match> captures;
            int distance = 0;
        };

        Scanner(std::string_view pattern);

        bool valid() const;
        ScanResult scan(bool allow_multiple = false, bool start_from_last = false);

    private:
        void init();
        bool parse_pattern(std::string_view pattern);

        size_t sign_bytes_size = 0;
        bool valid_ = false;
        std::vector<uint8_t> sign_bytes;
        std::vector<uint8_t> sign_mask;
        std::vector<Match> captures;

        static unsigned char* s_module_base;
        static size_t s_base_size;
        static unsigned char* s_last_address;
    };
}
#endif //ISAACSPY_SCANNER_H
