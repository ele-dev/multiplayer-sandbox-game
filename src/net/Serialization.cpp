#include "net/Serialization.hpp"

#include <cstring>

namespace game {
namespace {

template <typename T>
void append(std::vector<std::uint8_t>& bytes, const T& value) {
    const auto* raw = reinterpret_cast<const std::uint8_t*>(&value);
    bytes.insert(bytes.end(), raw, raw + sizeof(T));
}

template <typename T>
bool read(const std::vector<std::uint8_t>& bytes, std::size_t& offset, T& value) {
    if (offset + sizeof(T) > bytes.size()) {
        return false;
    }
    std::memcpy(&value, bytes.data() + offset, sizeof(T));
    offset += sizeof(T);
    return true;
}

std::vector<std::uint8_t> beginPacket(PacketType type) {
    std::vector<std::uint8_t> bytes;
    append(bytes, protocolVersion);
    append(bytes, type);
    return bytes;
}

bool readHeader(const std::vector<std::uint8_t>& bytes, PacketType expectedType, std::size_t& offset) {
    std::uint32_t version = 0;
    PacketType type = PacketType::Disconnect;
    return read(bytes, offset, version) && version == protocolVersion && read(bytes, offset, type) && type == expectedType;
}

} // namespace

std::vector<std::uint8_t> serializeClientHello(std::uint32_t sequence) {
    auto bytes = beginPacket(PacketType::ClientHello);
    append(bytes, sequence);
    return bytes;
}

std::vector<std::uint8_t> serializeClientInput(const ClientInputCommand& input) {
    auto bytes = beginPacket(PacketType::ClientInput);
    append(bytes, input.sequence);
    append(bytes, input.clientTick);
    append(bytes, input.movement.x);
    append(bytes, input.movement.y);
    append(bytes, input.lookDelta.x);
    append(bytes, input.lookDelta.y);
    return bytes;
}

std::vector<std::uint8_t> serializeDisconnect(std::uint32_t sequence) {
    auto bytes = beginPacket(PacketType::Disconnect);
    append(bytes, sequence);
    return bytes;
}

std::vector<std::uint8_t> serializeServerWelcome(std::uint32_t sequence, std::uint32_t playerId) {
    auto bytes = beginPacket(PacketType::ServerWelcome);
    append(bytes, sequence);
    append(bytes, playerId);
    return bytes;
}

std::vector<std::uint8_t> serializeServerSnapshot(const ServerSnapshot& snapshot) {
    auto bytes = beginPacket(PacketType::ServerSnapshot);
    append(bytes, snapshot.sequence);
    append(bytes, snapshot.serverTick);
    append(bytes, snapshot.player.playerId);
    append(bytes, snapshot.player.position.x);
    append(bytes, snapshot.player.position.y);
    append(bytes, snapshot.player.position.z);
    append(bytes, snapshot.player.yawRadians);
    append(bytes, snapshot.player.pitchRadians);
    return bytes;
}

std::optional<PacketType> readPacketType(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    std::uint32_t version = 0;
    PacketType type = PacketType::Disconnect;
    if (!read(bytes, offset, version) || version != protocolVersion || !read(bytes, offset, type)) {
        return std::nullopt;
    }
    return type;
}

std::optional<ClientHello> deserializeClientHello(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    if (!readHeader(bytes, PacketType::ClientHello, offset)) {
        return std::nullopt;
    }

    ClientHello hello;
    if (!read(bytes, offset, hello.sequence)) {
        return std::nullopt;
    }
    return hello;
}

std::optional<ClientInputCommand> deserializeClientInput(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    if (!readHeader(bytes, PacketType::ClientInput, offset)) {
        return std::nullopt;
    }

    ClientInputCommand input;
    if (!read(bytes, offset, input.sequence) || !read(bytes, offset, input.clientTick) ||
        !read(bytes, offset, input.movement.x) || !read(bytes, offset, input.movement.y) ||
        !read(bytes, offset, input.lookDelta.x) || !read(bytes, offset, input.lookDelta.y)) {
        return std::nullopt;
    }
    return input;
}

std::optional<Disconnect> deserializeDisconnect(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    if (!readHeader(bytes, PacketType::Disconnect, offset)) {
        return std::nullopt;
    }

    Disconnect disconnect;
    if (!read(bytes, offset, disconnect.sequence)) {
        return std::nullopt;
    }
    return disconnect;
}

std::optional<ServerWelcome> deserializeServerWelcome(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    if (!readHeader(bytes, PacketType::ServerWelcome, offset)) {
        return std::nullopt;
    }

    ServerWelcome welcome;
    if (!read(bytes, offset, welcome.sequence) || !read(bytes, offset, welcome.playerId)) {
        return std::nullopt;
    }
    return welcome;
}

std::optional<ServerSnapshot> deserializeServerSnapshot(const std::vector<std::uint8_t>& bytes) {
    std::size_t offset = 0;
    if (!readHeader(bytes, PacketType::ServerSnapshot, offset)) {
        return std::nullopt;
    }

    ServerSnapshot snapshot;
    if (!read(bytes, offset, snapshot.sequence) || !read(bytes, offset, snapshot.serverTick) ||
        !read(bytes, offset, snapshot.player.playerId) || !read(bytes, offset, snapshot.player.position.x) ||
        !read(bytes, offset, snapshot.player.position.y) || !read(bytes, offset, snapshot.player.position.z) ||
        !read(bytes, offset, snapshot.player.yawRadians) || !read(bytes, offset, snapshot.player.pitchRadians)) {
        return std::nullopt;
    }
    return snapshot;
}

} // namespace game
