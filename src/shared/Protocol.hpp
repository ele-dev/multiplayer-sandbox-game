#pragma once

#include "shared/Math.hpp"
#include "shared/PlayerState.hpp"

#include <cstdint>

namespace game {

constexpr std::uint32_t protocolVersion = 1;
constexpr std::uint16_t defaultServerPort = 27015;

enum class PacketType : std::uint8_t {
    ClientHello = 1,
    ServerWelcome = 2,
    ClientInput = 3,
    ServerSnapshot = 4,
    Disconnect = 5,
};

struct ClientInputCommand {
    std::uint32_t sequence = 0;
    std::uint64_t clientTick = 0;
    Vec2 movement = {};
    Vec2 lookDelta = {};
};

struct ServerSnapshot {
    std::uint32_t sequence = 0;
    std::uint64_t serverTick = 0;
    PlayerState player = {};
};

} // namespace game
