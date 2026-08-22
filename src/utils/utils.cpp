//
// Created by TsCat on 2026/6/24.
//

#include <cstdint>
#include <iostream>
#include <optional>
#include <ostream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <qrcodegen.hpp>

namespace utils
{
    void print_qrcode(const qrcodegen::QrCode& qr, int margin = 1) {
        auto set_color = [](int fr, int fg, int fb, int br, int bg, int bb)
        {
            std::cout << "\033[38;2;" << fr << ";" << fg << ";" << fb << "m"
                    << "\033[48;2;" << br << ";" << bg << ";" << bb << "m";
        };
        auto reset_color = [] { std::cout << "\033[0m"; };

        int size = qr.getSize();
        int matrix_size = size + 2 * margin;

        std::vector matrix(matrix_size, std::vector(matrix_size, 0));
        for (int y = 0; y < matrix_size; ++y) {
            for (int x = 0; x < matrix_size; ++x) {
                if (x >= margin && x < margin + size && y >= margin && y < margin + size)
                    matrix[y][x] = qr.getModule(x - margin, y - margin) ? 1 : 0;
                else
                    matrix[y][x] = 0;
            }
        }

        for (int y = 0; y < matrix_size; y += 2) {
            for (int x = 0; x < matrix_size; ++x) {
                int up = matrix[y][x];
                int down = (y + 1 < matrix_size) ? matrix[y + 1][x] : 0;

                if (up == 1 && down == 1) {
                    set_color(0, 0, 0, 0, 0, 0);
                    std::cout << "█";
                }
                else if (up == 1 && down == 0) {
                    set_color(0, 0, 0, 255, 255, 255);
                    std::cout << "▀";
                }
                else if (up == 0 && down == 1) {
                    set_color(0, 0, 0, 255, 255, 255);
                    std::cout << "▄";
                }
                else {
                    set_color(255, 255, 255, 255, 255, 255);
                    std::cout << " ";
                }
            }
            reset_color();
            std::cout << std::endl;
        }
        reset_color();
    }

    std::optional<std::string> get_internal_ip() {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            return std::nullopt;
        }

        struct WinsockResources {
            SOCKET socket_handle = INVALID_SOCKET;

            ~WinsockResources() {
                if (socket_handle != INVALID_SOCKET) {
                    closesocket(socket_handle);
                }
                WSACleanup();
            }
        } resources;

        resources.socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (resources.socket_handle == INVALID_SOCKET) {
            return std::nullopt;
        }

        std::vector<INTERFACE_INFO> interfaces(16);
        DWORD bytes_returned = 0;
        while (WSAIoctl(resources.socket_handle, SIO_GET_INTERFACE_LIST, nullptr, 0,
                        interfaces.data(), interfaces.size() * sizeof(INTERFACE_INFO),
                        &bytes_returned, nullptr, nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAEFAULT || interfaces.size() >= 1024) {
                return std::nullopt;
            }
            interfaces.resize(interfaces.size() * 2);
        }

        std::optional<std::string> best_ip;
        int best_priority = 4;

        const size_t interface_count = bytes_returned / sizeof(INTERFACE_INFO);
        for (size_t i = 0; i < interface_count; ++i) {
            const auto& interface_info = interfaces[i];
            if ((interface_info.iiFlags & IFF_UP) == 0 || (interface_info.iiFlags & IFF_LOOPBACK) != 0) {
                continue;
            }

            const auto* address = reinterpret_cast<const sockaddr_in*>(&interface_info.iiAddress);
            const uint32_t host_address = ntohl(address->sin_addr.s_addr);

            // Ignore this-network, loopback, link-local, multicast and reserved addresses.
            if ((host_address & 0xff000000u) == 0 ||
                (host_address & 0xff000000u) == 0x7f000000u ||
                (host_address & 0xffff0000u) == 0xa9fe0000u ||
                (host_address & 0xf0000000u) == 0xe0000000u ||
                (host_address & 0xf0000000u) == 0xf0000000u) {
                continue;
            }

            int priority = 3;
            if ((host_address & 0xffff0000u) == 0xc0a80000u) {
                priority = 0;
            }
            else if ((host_address & 0xff000000u) == 0x0a000000u) {
                priority = 1;
            }
            else if ((host_address & 0xfff00000u) == 0xac100000u) {
                priority = 2;
            }

            if (priority >= best_priority) {
                continue;
            }

            char ip_string[INET_ADDRSTRLEN]{};
            if (inet_ntop(AF_INET, &address->sin_addr, ip_string, sizeof(ip_string)) == nullptr) {
                continue;
            }

            if (priority == 0) {
                return std::string(ip_string);
            }

            best_priority = priority;
            best_ip = ip_string;
        }

        return best_ip;
        #else
        return std::nullopt;
        #endif
    }
}
