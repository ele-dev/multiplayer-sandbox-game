#include "client/ConnectingLayer.hpp"

#include <imgui.h>

namespace game {

ConnectingLayer::ConnectingLayer(const std::string& host, std::uint16_t port)
    : host_(host)
    , port_(port)
{
    setBlocking(true);
}

void ConnectingLayer::onRender() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 center = viewport->GetCenter();

    constexpr ImGuiWindowFlags connectingFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.35f);
    if (ImGui::Begin("Connecting", nullptr, connectingFlags)) {
        const float t = (float)ImGui::GetTime();
        const int dots = static_cast<int>(t * 3.0f) % 4;
        const char* suffix = "..........";
        suffix += 3 - dots;

        ImGui::Text("Connecting to %s:%u%s", host_.c_str(), port_, suffix);

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        const float spinnerSize = 18.0f;
        const float spinnerRadius = spinnerSize * 0.4f;
        const ImVec2 spinnerCenter = ImVec2(
            ImGui::GetWindowPos().x + ImGui::GetWindowWidth() / 2.0f,
            ImGui::GetCursorScreenPos().y + spinnerSize * 0.5f
        );
        const float angle = t * 3.0f;
        ImGui::GetWindowDrawList()->AddCircle(
            spinnerCenter, spinnerRadius, ImGui::GetColorU32(ImGuiCol_Text), 24, spinnerSize
        );
        ImGui::GetWindowDrawList()->AddCircleFilled(
            ImVec2(
                spinnerCenter.x + cosf(angle) * spinnerRadius,
                spinnerCenter.y + sinf(angle) * spinnerRadius
            ),
            3.0f,
            ImGui::GetColorU32(ImGuiCol_Text)
        );

        ImGui::Dummy(ImVec2(0.0f, 6.0f));
    }
    ImGui::End();
}

} // namespace game
