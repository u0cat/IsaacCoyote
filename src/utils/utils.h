//
// Created by TsCat on 2026/6/24.
//

#ifndef ISAACCOYOTE_UTILS_H
#define ISAACCOYOTE_UTILS_H

#include <optional>
#include <string>

#include <qrcodegen.hpp>

namespace utils
{
    std::optional<std::string> get_internal_ip();
    void print_qrcode(const qrcodegen::QrCode& qr, int margin = 1);
}
#endif // ISAACCOYOTE_UTILS_H
