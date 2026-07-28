#pragma once

#include "shared/Protocol.hpp"

#include <cstdint>

namespace game {

struct RenderDebugState {
    float frameTimeMs = 0.0f;
    float framesPerSecond = 0.0f;
    bool connected = false;
    std::uint32_t snapshotSequence = 0;
    std::uint64_t serverTick = 0;
    PlayerState player = {};
};

} // namespace game
