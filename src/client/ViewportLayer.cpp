#include "client/ViewportLayer.hpp"

#include "net/Serialization.hpp"

#include <iostream>
#include <cstdio>

namespace game {

ViewportLayer::ViewportLayer(
    Input& input,
    Camera& camera,
    RenderDebugState& debugState,
    OpenGLRenderer& renderer,
    UdpTransport& transport,
    const NetworkEndpoint& serverEndpoint,
    bool& isPaused,
    PauseToggleCallback onPauseToggle,
    ReturnToStartCallback onReturnToStart
)
    : input_(input)
    , camera_(camera)
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
    const auto command = input_.command(++inputSequence_, ++clientTick_);
    transport_.sendTo(serverEndpoint_, serializeClientInput(command));

    processNetwork();

    renderer_.render(camera_);
}

void ViewportLayer::onEvent(Event& event) {
    if (event.type == EventType::KeyDown && event.key.scancode == SDL_SCANCODE_ESCAPE) {
        printf("[ViewportLayer] ESC key handled, toggling pause from %d to %d\n", isPaused_, !isPaused_);
        isPaused_ = !isPaused_;
        if (onPauseToggle_) {
            onPauseToggle_(isPaused_);
        }
    }
}

bool ViewportLayer::wantsRelativeMouse() const {
    return !isPaused_;
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
