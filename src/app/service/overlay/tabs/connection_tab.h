//
// Created by TsCat on 2026/7/22.
//

#ifndef ISAACCOYOTE_CONNECTION_TAB_H
#define ISAACCOYOTE_CONNECTION_TAB_H
#include <qrcodegen.hpp>

#include "app/service/overlay/types.h"
#include "isaac_spy/isaac/game.h"

namespace app::coyote
{
    class CoyoteService;
}

namespace app::overlay::tabs
{
    class ConnectionTab : public ITab {
    public:
        explicit ConnectionTab() {} ;
        std::string get_id() override { return "Connection"; };
        std::string get_display_name() override { return "连接"; };
        void render() override;

    private:
        void draw_overview(coyote::CoyoteService* coyote_service);

        std::string ws_address_ = "";
        qrcodegen::QrCode ws_qr_code_ = qrcodegen::QrCode::encodeText("", qrcodegen::QrCode::Ecc::MEDIUM);
    };
}
#endif //ISAACCOYOTE_CONNECTION_TAB_H
