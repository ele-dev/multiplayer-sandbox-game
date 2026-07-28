#pragma once

#include "shared/Protocol.hpp"

#include <cstdint>

namespace game {

struct RenderDebugState {
    bool connected = false;
    std::uint32_t snapshotSequence = 0;
    std::uint64_t serverTick = 0;
    PlayerState player = {};
};

} // namespace game
