#include "client/MainMenuLayer.hpp"

#include <imgui.h>

namespace game {

MainMenuLayer::MainMenuLayer(ConnectCallback onConnect, ExitCallback onExit)
    : onConnect_(std::move(onConnect)), onExit_(std::move(onExit)) {}

void MainMenuLayer::onAttach() {
    activePage_ = Page::Start;
    resetConnectPage();
}

void MainMenuLayer::onRender() {
    if (activePage_ == Page::Connect) {
        renderConnectPage();
        return;
    }

    renderStartPage();
}

void MainMenuLayer::renderStartPage() {
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
            activePage_ = Page::Connect;
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

void MainMenuLayer::renderConnectPage() {
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
            activePage_ = Page::Start;
        }

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

void MainMenuLayer::resetConnectPage() {
    errorMessage_.clear();
    const std::string defaultIp = "127.0.0.1";
    defaultIp.copy(ipBuffer_, defaultIp.size());
    ipBuffer_[defaultIp.size()] = '\0';
}

} // namespace game
