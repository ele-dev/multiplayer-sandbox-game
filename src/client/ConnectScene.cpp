#include "client/ConnectScene.hpp"

#include <imgui.h>

namespace game {

ConnectScene::ConnectScene(ConnectCallback onConnect, BackCallback onBack)
    : onConnect_(std::move(onConnect)), onBack_(std::move(onBack)) {}

void ConnectScene::onEnter() {
    errorMessage_.clear();
    ipBuffer_[0] = '\0';
    const std::string defaultIp = "127.0.0.1";
    for (std::size_t i = 0; i < defaultIp.size(); ++i) {
        ipBuffer_[i] = defaultIp[i];
    }
    ipBuffer_[defaultIp.size()] = '\0';
}

void ConnectScene::renderImGui() {
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

    if (ImGui::Begin("Connect", nullptr, flags)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        ImGui::TextUnformatted("Server Address");
        ImGui::PushItemWidth(220.0f);
        ImGui::InputText("##ip", ipBuffer_, sizeof(ipBuffer_));
        ImGui::PopItemWidth();

        if (!errorMessage_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextUnformatted(errorMessage_.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.30f, 0.50f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.42f, 0.65f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.20f, 0.38f, 1.00f));

        if (ImGui::Button("Connect", ImVec2(220.0f, 0.0f))) {
            std::string ip(ipBuffer_);
            if (ip.empty()) {
                errorMessage_ = "Please enter an IP address";
            } else {
                errorMessage_.clear();
                if (onConnect_) {
                    onConnect_(ip);
                }
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 4.0f));

        if (ImGui::Button("Back", ImVec2(220.0f, 0.0f))) {
            if (onBack_) {
                onBack_();
            }
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

} // namespace game
