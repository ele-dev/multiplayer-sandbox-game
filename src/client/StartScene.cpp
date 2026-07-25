#include "client/StartScene.hpp"

#include <imgui.h>

namespace game {

StartScene::StartScene(PlayCallback onPlay, ExitCallback onExit)
    : onPlay_(std::move(onPlay)), onExit_(std::move(onExit)) {}

void StartScene::renderImGui() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.85f);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoNav;

    if (ImGui::Begin("Start Screen", nullptr, flags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 12.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.30f, 0.50f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.42f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.20f, 0.38f, 1.00f));

        ImGui::Dummy(ImVec2(0.0f, 8.0f));
        ImGui::TextUnformatted("MULTIPLAYER GAME");

        ImGui::Dummy(ImVec2(0.0f, 24.0f));

        if (ImGui::Button("Play", ImVec2(220.0f, 0.0f))) {
            if (onPlay_) {
                onPlay_();
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        if (ImGui::Button("Exit", ImVec2(220.0f, 0.0f))) {
            if (onExit_) {
                onExit_();
            }
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

} // namespace game
