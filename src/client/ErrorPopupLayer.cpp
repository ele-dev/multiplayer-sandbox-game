#include "client/ErrorPopupLayer.hpp"

#include <imgui.h>

namespace game {

ErrorPopupLayer::ErrorPopupLayer(CloseCallback onClose)
    : onClose_(std::move(onClose))
{
    setBlocking(true);
}

void ErrorPopupLayer::onRender() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Connection failed", nullptr, flags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 12.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.30f, 0.50f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.42f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.20f, 0.38f, 1.00f));

        ImGui::Text("Could not connect to server. Please check the address and try again.");

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        if (ImGui::Button("OK", ImVec2(220.0f, 0.0f))) {
            if (onClose_) {
                onClose_();
            }
        }
        ImGui::SetItemDefaultFocus();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

} // namespace game
