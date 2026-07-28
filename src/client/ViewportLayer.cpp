#include "client/ViewportLayer.hpp"

#include "net/Serialization.hpp"

#include <SDL3/SDL_scancode.h>

#include <iostream>

namespace game {

ViewportLayer::ViewportLayer(
    Camera& camera,
    RenderDebugState& debugState,
    OpenGLRenderer& renderer,
    UdpTransport& transport,
    const NetworkEndpoint& serverEndpoint,
    bool& isPaused,
    PauseToggleCallback onPauseToggle,
    ReturnToStartCallback onReturnToStart
)
    : camera_(camera)
    , debugState_(debugState)
    , renderer_(renderer)
    , transport_(transport)
    , serverEndpoint_(serverEndpoint)
    , isPaused_(isPaused)
    , onPauseToggle_(std::move(onPauseToggle))
    , onReturnToStart_(std::move(onReturnToStart)) {}

void ViewportLayer::onAttach() {
    isPaused_ = false;
    disconnectSent_ = false;
    forward_ = false;
    backward_ = false;
    left_ = false;
    right_ = false;
    lookUp_ = false;
    lookDown_ = false;
    lookLeft_ = false;
    lookRight_ = false;
    lookDelta_ = {};
    inputSequence_ = 0;
    clientTick_ = 0;
}

void ViewportLayer::onDetach() {
    if (!disconnectSent_) {
        transport_.sendTo(serverEndpoint_, serializeDisconnect(++inputSequence_));
        disconnectSent_ = true;
    }
    transport_.close();
    debugState_.connected = false;
}

void ViewportLayer::onUpdate() {
    const auto inputCommand = command(++inputSequence_, ++clientTick_);
    transport_.sendTo(serverEndpoint_, serializeClientInput(inputCommand));
    lookDelta_ = {};

    processNetwork();

    renderer_.render(camera_);
}

void ViewportLayer::onEvent(Event& event) {
    if (event.type == EventType::KeyDown && event.key.scancode == SDL_SCANCODE_ESCAPE) {
        isPaused_ = !isPaused_;
        if (onPauseToggle_) {
            onPauseToggle_(isPaused_);
        }
        event.handled = true;
        return;
    }

    if (event.type == EventType::MouseMove) {
        lookDelta_.x += event.mouseMove.xrel;
        lookDelta_.y -= event.mouseMove.yrel;
        event.handled = true;
        return;
    }

    if (event.type != EventType::KeyDown && event.type != EventType::KeyUp) {
        return;
    }

    const bool pressed = event.type == EventType::KeyDown;
    switch (event.key.scancode) {
    case SDL_SCANCODE_W:
        forward_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_S:
        backward_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_A:
        left_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_D:
        right_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_UP:
        lookUp_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_DOWN:
        lookDown_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_LEFT:
        lookLeft_ = pressed;
        event.handled = true;
        break;
    case SDL_SCANCODE_RIGHT:
        lookRight_ = pressed;
        event.handled = true;
        break;
    default:
        break;
    }
}

bool ViewportLayer::wantsRelativeMouse() const {
    return !isPaused_;
}

ClientInputCommand ViewportLayer::command(std::uint32_t sequence, std::uint64_t clientTick) const {
    constexpr float keyboardLookDeltaPerFrame = 8.0f;

    ClientInputCommand result;
    result.sequence = sequence;
    result.clientTick = clientTick;
    result.movement.x = (right_ ? 1.0f : 0.0f) - (left_ ? 1.0f : 0.0f);
    result.movement.y = (forward_ ? 1.0f : 0.0f) - (backward_ ? 1.0f : 0.0f);
    result.lookDelta = lookDelta_;
    result.lookDelta.x += ((lookRight_ ? 1.0f : 0.0f) - (lookLeft_ ? 1.0f : 0.0f)) * keyboardLookDeltaPerFrame;
    result.lookDelta.y += ((lookUp_ ? 1.0f : 0.0f) - (lookDown_ ? 1.0f : 0.0f)) * keyboardLookDeltaPerFrame;
    return result;
}

void ViewportLayer::processNetwork() {
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
