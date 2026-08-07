#pragma once

#include "net/GameNetworkingSocketsTransport.hpp"
#include "shared/Config.hpp"
#include "shared/Simulation.hpp"

#include <chrono>
#include <cstdint>
#include <optional>

namespace game {

class ServerApplication {
public:
    int run();

private:
    void loadConfig();
    void processNetwork();
    void checkClientTimeout(std::chrono::steady_clock::time_point now);
    void sendSnapshot();

    Config config_;
    GameNetworkingSocketsTransport transport_;
    Simulation simulation_;
    std::optional<NetworkEndpoint> clientEndpoint_;
    std::chrono::steady_clock::time_point lastClientPacketTime_ = std::chrono::steady_clock::now();
    std::uint32_t snapshotSequence_ = 0;
    std::uint16_t port_ = 27015;
    double tickSeconds_ = 1.0 / 60.0;
    std::chrono::seconds clientTimeout_ = std::chrono::seconds(5);
    std::chrono::seconds logInterval_ = std::chrono::seconds(5);
};

} // namespace game
