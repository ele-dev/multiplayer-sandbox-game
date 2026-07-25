#pragma once

#include "client/Camera.hpp"
#include "client/Event.hpp"
#include "client/GuiLayer.hpp"
#include "client/Input.hpp"
#include "client/Layer.hpp"
#include "client/OpenGLRenderer.hpp"
#include "net/UdpTransport.hpp"
#include "shared/Protocol.hpp"

#include <cstdint>
#include <functional>

namespace game {

class ViewportLayer : public Layer {
public:
    using PauseToggleCallback = std::function<void(bool paused)>;
    using ReturnToStartCallback = std::function<void()>;

    ViewportLayer(
        Input& input,
        Camera& camera,
        RenderDebugState& debugState,
        OpenGLRenderer& renderer,
        UdpTransport& transport,
        const NetworkEndpoint& serverEndpoint,
        bool& isPaused,
        PauseToggleCallback onPauseToggle,
        ReturnToStartCallback onReturnToStart
    );

    void onAttach() override;
    void onDetach() override;
    void onUpdate() override;
    void onEvent(Event& event) override;

    bool wantsRelativeMouse() const override;

private:
    void processNetwork();

    Input& input_;
    Camera& camera_;
    RenderDebugState& debugState_;
    OpenGLRenderer& renderer_;
    UdpTransport& transport_;
    NetworkEndpoint serverEndpoint_;
    bool& isPaused_;
    PauseToggleCallback onPauseToggle_;
    ReturnToStartCallback onReturnToStart_;
    bool disconnectSent_ = false;
    std::uint32_t inputSequence_ = 0;
    std::uint64_t clientTick_ = 0;
};

} // namespace game
