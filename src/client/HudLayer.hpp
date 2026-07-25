#pragma once

#include "client/Layer.hpp"

#include <imgui.h>

namespace game {

class HudLayer : public Layer {
public:
    explicit HudLayer(const bool& isPaused)
        : isPaused_(isPaused)
    {
        setBlocking(false);
    }

    void onRender() override {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 center = viewport->GetCenter();
        ImDrawList* foreground = ImGui::GetForegroundDrawList();

        if (!isPaused_) {
            const ImU32 crosshairColor = IM_COL32(215, 242, 255, 230);
            foreground->AddLine(ImVec2(center.x - 14.0f, center.y), ImVec2(center.x - 4.0f, center.y), crosshairColor, 1.5f);
            foreground->AddLine(ImVec2(center.x + 4.0f, center.y), ImVec2(center.x + 14.0f, center.y), crosshairColor, 1.5f);
            foreground->AddLine(ImVec2(center.x, center.y - 14.0f), ImVec2(center.x, center.y - 4.0f), crosshairColor, 1.5f);
            foreground->AddLine(ImVec2(center.x, center.y + 4.0f), ImVec2(center.x, center.y + 14.0f), crosshairColor, 1.5f);
        }
    }

private:
    const bool& isPaused_;
};

} // namespace game
