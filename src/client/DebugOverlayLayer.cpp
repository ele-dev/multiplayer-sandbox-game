#include "client/DebugOverlayLayer.hpp"

#include <imgui.h>

namespace game {

void DebugOverlayLayer::onRender() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

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
}

} // namespace game
