#pragma once

#include "net/NetworkTransport.hpp"

#include <cstdint>

namespace game {

class UdpTransport final : public NetworkTransport {
public:
    UdpTransport();
    ~UdpTransport() override;

    UdpTransport(const UdpTransport&) = delete;
    UdpTransport& operator=(const UdpTransport&) = delete;

    bool open(std::uint16_t localPort) override;
    void close() override;
    bool sendTo(const NetworkEndpoint& endpoint, const std::vector<std::uint8_t>& bytes) override;
    std::optional<NetworkPacket> receive() override;

private:
    std::intptr_t socket_ = -1;
};

} // namespace game
