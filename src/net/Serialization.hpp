#pragma once

#include "shared/Protocol.hpp"

#include <optional>
#include <vector>

namespace game {

std::vector<std::uint8_t> serializeClientHello(std::uint32_t sequence);
std::vector<std::uint8_t> serializeClientInput(const ClientInputCommand& input);
std::vector<std::uint8_t> serializeDisconnect(std::uint32_t sequence);
std::vector<std::uint8_t> serializeServerWelcome(std::uint32_t sequence, std::uint32_t playerId);
std::vector<std::uint8_t> serializeServerSnapshot(const ServerSnapshot& snapshot);

std::optional<PacketType> readPacketType(const std::vector<std::uint8_t>& bytes);
std::optional<ClientInputCommand> deserializeClientInput(const std::vector<std::uint8_t>& bytes);
std::optional<ServerSnapshot> deserializeServerSnapshot(const std::vector<std::uint8_t>& bytes);

} // namespace game
