#include "client/GuiLayer.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace game {

GuiLayer::~GuiLayer() {
    shutdown();
}

bool GuiLayer::initialize(SDL_Window* window, SDL_GLContext glContext) {
    if (initialized_) {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 1.0f;

    if (!ImGui_ImplSDL3_InitForOpenGL(window, glContext)) {
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    initialized_ = true;
    return true;
}

void GuiLayer::processEvent(const SDL_Event& event) {
    if (!initialized_) {
        return;
    }
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void GuiLayer::render(const RenderDebugState& debugState) {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();
    ImDrawList* foreground = ImGui::GetForegroundDrawList();
    const ImU32 crosshairColor = IM_COL32(215, 242, 255, 230);
    foreground->AddLine(ImVec2(center.x - 14.0f, center.y), ImVec2(center.x - 4.0f, center.y), crosshairColor, 1.5f);
    foreground->AddLine(ImVec2(center.x + 4.0f, center.y), ImVec2(center.x + 14.0f, center.y), crosshairColor, 1.5f);
    foreground->AddLine(ImVec2(center.x, center.y - 14.0f), ImVec2(center.x, center.y - 4.0f), crosshairColor, 1.5f);
    foreground->AddLine(ImVec2(center.x, center.y + 4.0f), ImVec2(center.x, center.y + 14.0f), crosshairColor, 1.5f);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                  ImGuiWindowFlags_NoNav;
    const ImVec2 windowPosition = ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 16.0f);
    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.62f);
    if (ImGui::Begin("Network Debug", nullptr, flags)) {
        ImGui::TextUnformatted("Network Debug");
        ImGui::Separator();
        ImGui::Text("connected: %s", debugState.connected ? "yes" : "no");
        ImGui::Text("server tick: %llu", static_cast<unsigned long long>(debugState.serverTick));
        ImGui::Text("snapshot: %u", debugState.snapshotSequence);
        ImGui::Text(
            "pos: %.2f %.2f %.2f",
            debugState.player.position.x,
            debugState.player.position.y,
            debugState.player.position.z
        );
        ImGui::Text("yaw/pitch: %.3f / %.3f", debugState.player.yawRadians, debugState.player.pitchRadians);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiLayer::shutdown() {
    if (!initialized_) {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

} // namespace game
