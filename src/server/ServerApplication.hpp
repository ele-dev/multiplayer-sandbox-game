#pragma once

#include "net/UdpTransport.hpp"
#include "shared/Simulation.hpp"

#include <chrono>
#include <optional>

namespace game {

class ServerApplication {
public:
    int run();

private:
    void processNetwork();
    void checkClientTimeout(std::chrono::steady_clock::time_point now);
    void sendSnapshot();
    [[nodiscard]] bool isCurrentClient(const NetworkEndpoint& endpoint) const;

    UdpTransport transport_;
    Simulation simulation_;
    std::optional<NetworkEndpoint> clientEndpoint_;
    std::chrono::steady_clock::time_point lastClientPacketTime_ = std::chrono::steady_clock::now();
    std::uint32_t snapshotSequence_ = 0;
};

} // namespace game
