#pragma once

#include "shared/Protocol.hpp"

#include <SDL3/SDL_events.h>

namespace game {

class Input {
public:
    void beginFrame();
    void handleEvent(const SDL_Event& event);

    [[nodiscard]] ClientInputCommand command(std::uint32_t sequence, std::uint64_t clientTick) const;
    [[nodiscard]] bool quitRequested() const;

private:
    bool quitRequested_ = false;
    bool forward_ = false;
    bool backward_ = false;
    bool left_ = false;
    bool right_ = false;
    bool lookUp_ = false;
    bool lookDown_ = false;
    bool lookLeft_ = false;
    bool lookRight_ = false;
    glm::vec2 lookDelta_ = {};
};

} // namespace game
