#pragma once

#include "client/Camera.hpp"
#include "client/Event.hpp"
#include "client/Layer.hpp"
#include "client/OpenGLRenderer.hpp"
#include "client/RenderDebugState.hpp"
#include "net/NetworkTransport.hpp"
#include "shared/Protocol.hpp"

#include <cstdint>
#include <functional>
#include <glm/vec2.hpp>

namespace game {

class ViewportLayer : public Layer {
public:
    using PauseToggleCallback = std::function<void(bool paused)>;
    using ReturnToStartCallback = std::function<void()>;

    ViewportLayer(
        Camera& camera,
        RenderDebugState& debugState,
        OpenGLRenderer& renderer,
        NetworkTransport& transport,
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
    ClientInputCommand buildInputCommand(std::uint32_t sequence, std::uint64_t clientTick) const;
    void processNetwork();

    Camera& camera_;
    RenderDebugState& debugState_;
    OpenGLRenderer& renderer_;
    NetworkTransport& transport_;
    NetworkEndpoint serverEndpoint_;
    bool& isPaused_;
    PauseToggleCallback onPauseToggle_;
    ReturnToStartCallback onReturnToStart_;
    bool disconnectSent_ = false;
    bool forward_ = false;
    bool backward_ = false;
    bool left_ = false;
    bool right_ = false;
    bool lookUp_ = false;
    bool lookDown_ = false;
    bool lookLeft_ = false;
    bool lookRight_ = false;
    glm::vec2 gamepadMove_ = {};
    glm::vec2 gamepadLook_ = {};
    glm::vec2 lookDelta_ = {};
    std::uint32_t inputSequence_ = 0;
    std::uint64_t clientTick_ = 0;
};

} // namespace game
