#include "client/InGameScene.hpp"

#include "net/Serialization.hpp"

#include <imgui.h>

#include <iostream>

namespace game {

InGameScene::InGameScene(
    Input& input,
    Camera& camera,
    RenderDebugState& debugState,
    UdpTransport& transport,
    const NetworkEndpoint& serverEndpoint,
    ReturnToStartCallback onReturnToStart
)
    : input_(input)
    , camera_(camera)
    , debugState_(debugState)
    , transport_(transport)
    , serverEndpoint_(serverEndpoint)
    , onReturnToStart_(std::move(onReturnToStart)) {}

void InGameScene::onEnter() {
    paused_ = false;
    disconnectSent_ = false;
    inputSequence_ = 0;
    clientTick_ = 0;
}

void InGameScene::onExit() {
    if (!disconnectSent_) {
        transport_.sendTo(serverEndpoint_, serializeDisconnect(++inputSequence_));
        disconnectSent_ = true;
    }
    transport_.close();
    debugState_.connected = false;
}

void InGameScene::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
        paused_ = !paused_;
    }
}

void InGameScene::update() {
    const auto command = input_.command(++inputSequence_, ++clientTick_);
    transport_.sendTo(serverEndpoint_, serializeClientInput(command));

    processNetwork();
}

void InGameScene::renderImGui() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();
    ImDrawList* foreground = ImGui::GetForegroundDrawList();

    if (!paused_) {
        const ImU32 crosshairColor = IM_COL32(215, 242, 255, 230);
        foreground->AddLine(ImVec2(center.x - 14.0f, center.y), ImVec2(center.x - 4.0f, center.y), crosshairColor, 1.5f);
        foreground->AddLine(ImVec2(center.x + 4.0f, center.y), ImVec2(center.x + 14.0f, center.y), crosshairColor, 1.5f);
        foreground->AddLine(ImVec2(center.x, center.y - 14.0f), ImVec2(center.x, center.y - 4.0f), crosshairColor, 1.5f);
        foreground->AddLine(ImVec2(center.x, center.y + 4.0f), ImVec2(center.x, center.y + 14.0f), crosshairColor, 1.5f);
    }

    constexpr ImGuiWindowFlags debugFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 16.0f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f)
    );
    ImGui::SetNextWindowBgAlpha(0.62f);
    if (ImGui::Begin("Network Debug", nullptr, debugFlags)) {
        ImGui::TextUnformatted("Network Debug");
        ImGui::Separator();
        ImGui::Text("connected: %s", debugState_.connected ? "yes" : "no");
        ImGui::Text("server tick: %llu", static_cast<unsigned long long>(debugState_.serverTick));
        ImGui::Text("snapshot: %u", debugState_.snapshotSequence);
        ImGui::Text(
            "pos: %.2f %.2f %.2f",
            debugState_.player.position.x,
            debugState_.player.position.y,
            debugState_.player.position.z
        );
        ImGui::Text("yaw/pitch: %.3f / %.3f", debugState_.player.yawRadians, debugState_.player.pitchRadians);
    }
    ImGui::End();

    if (paused_) {
        constexpr ImGuiWindowFlags pauseFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoNav;

        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.35f);
        if (ImGui::Begin("Pause", nullptr, pauseFlags)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 12.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.30f, 0.50f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.42f, 0.65f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.20f, 0.38f, 1.00f));

            if (ImGui::Button("Resume Game", ImVec2(220.0f, 0.0f))) {
                paused_ = false;
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
}

bool InGameScene::wantsRelativeMouse() const {
    return !paused_;
}

void InGameScene::processNetwork() {
    while (auto packet = transport_.receive()) {
        const auto type = readPacketType(packet->bytes);
        if (!type) {
            continue;
        }

        if (*type == PacketType::ServerWelcome) {
            debugState_.connected = true;
            std::cout << "connected to server\n";
            continue;
        }

        if (*type == PacketType::ServerSnapshot) {
            if (auto snapshot = deserializeServerSnapshot(packet->bytes)) {
                camera_.setFromPlayer(snapshot->player);
                debugState_.connected = true;
                debugState_.snapshotSequence = snapshot->sequence;
                debugState_.serverTick = snapshot->serverTick;
                debugState_.player = snapshot->player;
            }
        }
    }
}

} // namespace game
