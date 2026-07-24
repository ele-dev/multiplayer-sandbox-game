#pragma once

#include "shared/Math.hpp"

#include <cstdint>

namespace game {

struct PlayerState {
    std::uint32_t playerId = 0;
    Vec3 position = {0.0f, 1.8f, 0.0f};
    float yawRadians = 0.0f;
    float pitchRadians = 0.0f;
};

} // namespace game
