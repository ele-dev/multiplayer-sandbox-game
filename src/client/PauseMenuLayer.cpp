#include "client/PauseMenuLayer.hpp"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

#include <imgui.h>

namespace game {

PauseMenuLayer::PauseMenuLayer(ResumeCallback onResume, ReturnToStartCallback onReturnToStart, SettingsCallback onSettings)
    : onResume_(std::move(onResume))
    , onReturnToStart_(std::move(onReturnToStart))
    , settingsCallback_(std::move(onSettings))
{
    setBlocking(true);
}

void PauseMenuLayer::onEvent(Event& event) {
    if (event.type == EventType::KeyDown && event.key.scancode == SDL_SCANCODE_ESCAPE) {
        if (onResume_) {
            onResume_();
        }
        event.consumed = true;
        return;
    }

    if (event.type == EventType::GamepadButtonDown && event.gamepadButton.button == SDL_GAMEPAD_BUTTON_START) {
        if (onResume_) {
            onResume_();
        }
        event.consumed = true;
        return;
    }
}

void PauseMenuLayer::onRender() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();

    constexpr ImGuiWindowFlags pauseFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Pause", nullptr, pauseFlags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 12.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.30f, 0.50f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.42f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.20f, 0.38f, 1.00f));

        if (ImGui::Button("Resume Game", ImVec2(220.0f, 0.0f))) {
            if (onResume_) {
                onResume_();
            }
        }
        ImGui::SetItemDefaultFocus();

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        if (ImGui::Button("Settings", ImVec2(220.0f, 0.0f))) {
            if (settingsCallback_) {
                settingsCallback_();
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        if (ImGui::Button("Return to Start", ImVec2(220.0f, 0.0f))) {
            if (onReturnToStart_) {
                onReturnToStart_();
            }
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

} // namespace game
