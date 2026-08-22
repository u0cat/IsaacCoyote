//
// Created by TsCat on 2026/7/29.
//

#include "app/service/overlay/core/imgui_renderer.h"

#include <algorithm>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

#include <imgui.h>

#include "app/service/overlay/core/hook.h"
#include "app/service/overlay/ui/themes.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

using namespace app::overlay;

ImGuiRenderer::ImGuiRenderer(config::ConfigService& config, PlatformHook& platform)
    : config_(config), platform_(platform) {}

ImGuiRenderer::~ImGuiRenderer() {
    shutdown();
}

void ImGuiRenderer::shutdown() {
    if (!initialized_) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

bool ImGuiRenderer::begin_frame(const HDC hdc) {
    if (initialization_failed_) return false;
    if (!initialized_ && !initialize(hdc)) {
        initialization_failed_ = true;
        return false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    update_scale();
    return true;
}

void ImGuiRenderer::end_frame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::optional<LRESULT> ImGuiRenderer::handle_message(
    const HWND window, const UINT msg, const WPARAM wparam, const LPARAM lparam) const {
    if (initialized_) {
        if (const auto result = ImGui_ImplWin32_WndProcHandler(window, msg, wparam, lparam))
            return result;
    }
    return std::nullopt;
}

bool ImGuiRenderer::is_initialized() const noexcept {
    return initialized_;
}

bool ImGuiRenderer::initialize(const HDC hdc) {
    struct Guard {
        bool context = false;
        bool win32 = false;
        bool opengl = false;
        bool dismissed = false;

        ~Guard() {
            if (dismissed) return;
            if (opengl) ImGui_ImplOpenGL3_Shutdown();
            if (win32) ImGui_ImplWin32_Shutdown();
            if (context) ImGui::DestroyContext();
        }
    } guard;

    window_ = WindowFromDC(hdc);
    if (!window_ || !platform_.attach_window(window_)) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    guard.context = true;

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = ini_path_.c_str();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (!ImGui_ImplWin32_InitForOpenGL(window_)) return false;
    guard.win32 = true;
    if (!ImGui_ImplOpenGL3_Init()) return false;
    guard.opengl = true;

    if (ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simhei.ttf", 16.0f))
        io.FontDefault = font;

    initialized_ = true;
    guard.dismissed = true;
    update_scale();
    return true;
}

void ImGuiRenderer::update_scale() {
    if (!window_) return;

    const auto overlay = config_.snapshot()->overlay;
    const float scale = overlay.automatic_dpi ? 1.0f : std::clamp(overlay.scale, 0.75f, 2.5f);
    if (std::abs(applied_scale_ - scale) < 0.001f) return;

    ui::apply_theme(scale);
    ImGui::GetIO().FontGlobalScale = scale;
    applied_scale_ = scale;
}
