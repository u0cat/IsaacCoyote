//
// Created by TsCat on 2026/7/22.
//

#include "app/service/overlay/tabs/connection_tab.h"

#include <algorithm>
#include <cmath>

#include <imgui.h>
#include <qrcodegen.hpp>

#include "app/game.h"
#include "app/isaac_coyote.h"
#include "app/service/coyote/websocket/client.h"
#include "app/service/overlay/ui/components.h"

using namespace app::overlay::tabs;

namespace
{
    namespace ui = app::overlay::ui;

    void draw_qr_code(const qrcodegen::QrCode& qr_code) {
        const int size = qr_code.getSize();
        const float available = std::min(ImGui::GetContentRegionAvail().x,
                                         ui::metrics::scale(ui::metrics::kQrMaximumSize));
        const float modules = static_cast<float>(size) + ui::metrics::kQrQuietModules * 2.0f;
        const float module_size = available > 0.0f ? available / modules : 0.0f;
        const float qr_size = modules * module_size;

        ui::center_next_item(qr_size);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const ImVec2 code_origin(origin.x + ui::metrics::kQrQuietModules * module_size,
                                 origin.y + ui::metrics::kQrQuietModules * module_size);
        draw_list->AddRectFilled(origin, ImVec2(origin.x + qr_size, origin.y + qr_size), IM_COL32_WHITE,
                                 ui::metrics::scale(ui::metrics::kFrameRounding));
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr_code.getModule(x, y))
                    draw_list->AddRectFilled(
                        {code_origin.x + x * module_size, code_origin.y + y * module_size},
                        {code_origin.x + (x + 1) * module_size, code_origin.y + (y + 1) * module_size},
                        IM_COL32_BLACK);
            }
        }
        ImGui::Dummy(ImVec2(qr_size, qr_size));
    }
}

void ConnectionTab::draw_overview(coyote::CoyoteService* coyote_service) {
    auto* controller = coyote_service->get_ws_controller();
    if (!controller) return;

    if (ui::begin_surface_card("overview")) {
        ui::section_header("连接状态", "");
        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        if (ui::begin_property_table("overview_info")) {
            ui::property_row("客户端数量");
            ImGui::Text("%d", controller->client_ids().size());

            ui::property_row("总设备数量");
            int device_count = 0;
            for (const auto& client_id : controller->client_ids()) {
                if (auto client = controller->find_client(client_id))
                    device_count += client->alive_device_count();
            }
            ImGui::Text("%d", device_count);
        }
        ui::end_property_table();
    }
    ui::end_surface_card();
}

void ConnectionTab::render() {
    auto* coyote_service = IsaacCoyote::get_instance().get_coyote_service();
    if (!coyote_service) return;

    if (coyote_service->is_ready()) {
        const std::string address = coyote_service->get_ws_controller()->get_address();
        if (address != ws_address_) {
            ws_address_ = address;
            ws_qr_code_ = qrcodegen::QrCode::encodeText(
                std::format("https://dungeon-lab.cn/s/?v=1&action=socket&url={}", address).c_str(),
                qrcodegen::QrCode::Ecc::LOW
            );
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(ui::metrics::kSectionGap, ui::metrics::kPageGap));
    if (ImGui::BeginChild("conn_root", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_AlwaysUseWindowPadding,
                          ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        const bool compact = ImGui::GetContentRegionAvail().x < ui::metrics::scale(690.0f);
        if (ImGui::BeginTable("connection_master_detail", compact ? 1 : 3,
                              ImGuiTableFlags_SizingStretchProp)) {
            if (!compact) {
                ImGui::TableSetupColumn("view", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("gap", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize,
                                        ui::metrics::scale(ui::metrics::kColumnGap));
                ImGui::TableSetupColumn("qrcode", ImGuiTableColumnFlags_WidthFixed,
                                        ui::metrics::scale(400.0f));
            }

            ImGui::TableNextColumn();
            if (coyote_service->is_ready()) {
                if (ui::begin_surface_card("connection_status")) {
                    ui::status_chip("connected", "已连接服务器", ui::kSuccess);
                    ui::gap(ui::metrics::kSectionGap);
                    ui::title_text("等待设备连接......");
                    ui::gap(ui::metrics::kSectionGap);

                    ImGui::TextUnformatted("连接地址");
                    if (ui::begin_inset_panel("address")) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ui::kInfo);
                        ImGui::TextWrapped("%s", ws_address_.c_str());
                        ImGui::PopStyleColor();
                    }
                    ui::end_inset_panel();

                    ui::gap(ui::metrics::kSectionGap);
                    if (ui::danger_button("停止服务", ImVec2(-1.0f, ui::metrics::kButtonHeight)))
                        coyote_service->stop();
                }
                ui::end_surface_card();

                draw_overview(coyote_service);
            }
            else {
                if (ui::begin_surface_card("connection_idle")) {
                    ui::title_text("设备连接");
                    ui::gap(ui::metrics::kSmallGap);
                    ui::muted_text("启动服务后，用 DGLAB App 扫码配对。");
                    ui::gap(ui::metrics::kSectionGap);
                    const float button_width = std::min(ui::metrics::scale(240.0f),
                                                        ImGui::GetContentRegionAvail().x);
                    if (ui::primary_button("启动服务", ImVec2(button_width, ui::metrics::kButtonHeight)))
                        coyote_service->start();
                }
                ui::end_surface_card();
            }

            ImGui::TableNextColumn();
            if (!compact) ImGui::Dummy(ImVec2(ui::metrics::scale(ui::metrics::kColumnGap), 0.0f));

            if (!compact) ImGui::TableNextColumn();
            if (ui::begin_surface_card("pairing_qr")) {
                ui::title_text("扫码配对");
                ui::muted_text("打开 DGLAB 4，扫描下方二维码。");
                ui::gap(ui::metrics::kSectionGap);

                if (coyote_service->is_ready()) {
                    if (ws_qr_code_.getSize() > 0) {
                        draw_qr_code(ws_qr_code_);
                    }
                    else {
                        ImGui::TextColored(ui::kError, "二维码生成失败，请重启服务。");
                    }
                }
                else {
                    ui::muted_text("等待服务启动");
                    ui::gap(ui::metrics::kSectionGap);
                }
                ui::gap(ui::metrics::kSectionGap);
            }
            ui::end_surface_card();

            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
}
