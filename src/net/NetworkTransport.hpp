#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace game {

struct NetworkEndpoint {
    std::string host;
    std::uint16_t port = 0;
};

struct NetworkPacket {
    NetworkEndpoint from;
    std::vector<std::uint8_t> bytes;
};

class NetworkTransport {
public:
    virtual ~NetworkTransport() = default;

    virtual bool open(std::uint16_t localPort) = 0;
    virtual void close() = 0;
    virtual bool sendTo(const NetworkEndpoint& endpoint, const std::vector<std::uint8_t>& bytes) = 0;
    virtual std::optional<NetworkPacket> receive() = 0;
};

} // namespace game
