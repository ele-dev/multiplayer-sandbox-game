#include "client/ViewportLayer.hpp"

#include "net/Serialization.hpp"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_scancode.h>

#include <iostream>

namespace game {

namespace {

constexpr float gamepadDeadzone = 0.18f;
constexpr float gamepadLookDeltaPerFrame = 10.0f;

float applyDeadzone(float value) {
    if (value > -gamepadDeadzone && value < gamepadDeadzone) {
        return 0.0f;
    }
    return value;
}

} // namespace

ViewportLayer::ViewportLayer(
    Camera& camera,
    RenderDebugState& debugState,
    OpenGLRenderer& renderer,
    NetworkTransport& transport,
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
    gamepadMove_ = {};
    gamepadLook_ = {};
    lookDelta_ = {};
    inputSequence_ = 0;
    clientTick_ = 0;
}

void ViewportLayer::onDetach() {
    if (!disconnectSent_) {
        transport_.send(serializeDisconnect(++inputSequence_), NetworkSendMode::Reliable);
        disconnectSent_ = true;
    }
    transport_.close();
    debugState_.connected = false;
}

void ViewportLayer::onUpdate() {
    const auto inputCommand = buildInputCommand(++inputSequence_, ++clientTick_);
    transport_.send(serializeClientInput(inputCommand), NetworkSendMode::Unreliable);
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
        event.consumed = true;
        return;
    }

    if (event.type == EventType::GamepadButtonDown && event.gamepadButton.button == SDL_GAMEPAD_BUTTON_START) {
        isPaused_ = !isPaused_;
        if (onPauseToggle_) {
            onPauseToggle_(isPaused_);
        }
        event.consumed = true;
        return;
    }

    if (event.type == EventType::MouseMove) {
        lookDelta_.x += event.mouseMove.xrel;
        lookDelta_.y -= event.mouseMove.yrel;
        event.consumed = true;
        return;
    }

    if (event.type == EventType::GamepadAxisMotion) {
        const float value = applyDeadzone(event.gamepadAxis.value);
        switch (event.gamepadAxis.axis) {
        case SDL_GAMEPAD_AXIS_LEFTX:
            gamepadMove_.x = value;
            event.consumed = true;
            break;
        case SDL_GAMEPAD_AXIS_LEFTY:
            gamepadMove_.y = -value;
            event.consumed = true;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTX:
            gamepadLook_.x = value;
            event.consumed = true;
            break;
        case SDL_GAMEPAD_AXIS_RIGHTY:
            gamepadLook_.y = -value;
            event.consumed = true;
            break;
        default:
            break;
        }
        return;
    }

    if (event.type == EventType::GamepadRemoved) {
        gamepadMove_ = {};
        gamepadLook_ = {};
        event.consumed = true;
        return;
    }

    if (event.type != EventType::KeyDown && event.type != EventType::KeyUp) {
        return;
    }

    const bool pressed = event.type == EventType::KeyDown;
    switch (event.key.scancode) {
    case SDL_SCANCODE_W:
        forward_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_S:
        backward_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_A:
        left_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_D:
        right_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_UP:
        lookUp_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_DOWN:
        lookDown_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_LEFT:
        lookLeft_ = pressed;
        event.consumed = true;
        break;
    case SDL_SCANCODE_RIGHT:
        lookRight_ = pressed;
        event.consumed = true;
        break;
    default:
        break;
    }
}

bool ViewportLayer::wantsRelativeMouse() const {
    return !isPaused_;
}

ClientInputCommand ViewportLayer::buildInputCommand(std::uint32_t sequence, std::uint64_t clientTick) const {
    constexpr float keyboardLookDeltaPerFrame = 8.0f;

    ClientInputCommand result;
    result.sequence = sequence;
    result.clientTick = clientTick;
    result.movement.x = (right_ ? 1.0f : 0.0f) - (left_ ? 1.0f : 0.0f) + gamepadMove_.x;
    result.movement.y = (forward_ ? 1.0f : 0.0f) - (backward_ ? 1.0f : 0.0f) + gamepadMove_.y;
    result.lookDelta = lookDelta_;
    result.lookDelta.x += ((lookRight_ ? 1.0f : 0.0f) - (lookLeft_ ? 1.0f : 0.0f)) * keyboardLookDeltaPerFrame;
    result.lookDelta.y += ((lookUp_ ? 1.0f : 0.0f) - (lookDown_ ? 1.0f : 0.0f)) * keyboardLookDeltaPerFrame;
    result.lookDelta.x += gamepadLook_.x * gamepadLookDeltaPerFrame;
    result.lookDelta.y += gamepadLook_.y * gamepadLookDeltaPerFrame;
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
