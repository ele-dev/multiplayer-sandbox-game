#include "client/Input.hpp"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>

#include <cstdio>

namespace game {

void Input::beginFrame() {
    lookDelta_ = {};
}

void Input::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        printf("[Input] SDL_EVENT_QUIT received, setting quitRequested_\n");
        quitRequested_ = true;
        return;
    }

    if (event.type == SDL_EVENT_MOUSE_MOTION) {
        lookDelta_.x += event.motion.xrel;
        lookDelta_.y -= event.motion.yrel;
        return;
    }

    if (event.type != SDL_EVENT_KEY_DOWN && event.type != SDL_EVENT_KEY_UP) {
        return;
    }

    const bool pressed = event.type == SDL_EVENT_KEY_DOWN;
    switch (event.key.scancode) {
    case SDL_SCANCODE_W:
        forward_ = pressed;
        break;
    case SDL_SCANCODE_S:
        backward_ = pressed;
        break;
    case SDL_SCANCODE_A:
        left_ = pressed;
        break;
    case SDL_SCANCODE_D:
        right_ = pressed;
        break;
    case SDL_SCANCODE_UP:
        lookUp_ = pressed;
        break;
    case SDL_SCANCODE_DOWN:
        lookDown_ = pressed;
        break;
    case SDL_SCANCODE_LEFT:
        lookLeft_ = pressed;
        break;
    case SDL_SCANCODE_RIGHT:
        lookRight_ = pressed;
        break;
    default:
        break;
    }
}

ClientInputCommand Input::command(std::uint32_t sequence, std::uint64_t clientTick) const {
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

bool Input::quitRequested() const {
    return quitRequested_;
}

} // namespace game
