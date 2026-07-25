#pragma once

#include "client/Camera.hpp"
#include "client/GuiLayer.hpp"
#include "client/Input.hpp"
#include "client/Scene.hpp"
#include "net/UdpTransport.hpp"
#include "shared/Protocol.hpp"

#include <cstdint>
#include <functional>

namespace game {

class InGameScene : public Scene {
public:
    using ReturnToStartCallback = std::function<void()>;

    InGameScene(
        Input& input,
        Camera& camera,
        RenderDebugState& debugState,
        UdpTransport& transport,
        const NetworkEndpoint& serverEndpoint,
        ReturnToStartCallback onReturnToStart
    );

    void onEnter() override;
    void onExit() override;
    void handleEvent(const SDL_Event& event) override;
    void update() override;
    void renderImGui() override;

    bool wantsRelativeMouse() const override;
    bool wantsRender3D() const override { return true; }

private:
    void processNetwork();

    Input& input_;
    Camera& camera_;
    RenderDebugState& debugState_;
    UdpTransport& transport_;
    NetworkEndpoint serverEndpoint_;
    ReturnToStartCallback onReturnToStart_;
    bool paused_ = false;
    bool disconnectSent_ = false;
    std::uint32_t inputSequence_ = 0;
    std::uint64_t clientTick_ = 0;
};

} // namespace game
