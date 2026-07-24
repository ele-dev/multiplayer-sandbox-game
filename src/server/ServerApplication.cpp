#include "server/ServerApplication.hpp"

#include "net/Serialization.hpp"
#include "shared/Timestep.hpp"

#include <chrono>
#include <iostream>
#include <thread>

namespace game {

int ServerApplication::run() {
    std::cout << std::unitbuf;

    if (!transport_.open(defaultServerPort)) {
        std::cerr << "Failed to bind UDP server port " << defaultServerPort << '\n';
        return 1;
    }

    std::cout << "game_server listening on UDP port " << defaultServerPort << '\n';
    std::cout << "Press Ctrl+C to stop.\n";

    using clock = std::chrono::steady_clock;
    auto previous = clock::now();
    double accumulator = 0.0;
    auto lastLog = previous;

    while (true) {
        const auto now = clock::now();
        accumulator += std::chrono::duration<double>(now - previous).count();
        previous = now;

        processNetwork();
        checkClientTimeout(now);

        while (accumulator >= fixedTickSeconds) {
            simulation_.tick(static_cast<float>(fixedTickSeconds));
            sendSnapshot();
            accumulator -= fixedTickSeconds;
        }

        if (now - lastLog >= std::chrono::seconds(5)) {
            const auto& player = simulation_.player();
            std::cout << "tick=" << simulation_.tickCount() << " player=(" << player.position.x << ", "
                      << player.position.y << ", " << player.position.z << ")\n";
            lastLog = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ServerApplication::processNetwork() {
    while (auto packet = transport_.receive()) {
        const auto type = readPacketType(packet->bytes);
        if (!type) {
            continue;
        }

        if (*type == PacketType::ClientHello) {
            clientEndpoint_ = packet->from;
            lastClientPacketTime_ = std::chrono::steady_clock::now();
            transport_.sendTo(*clientEndpoint_, serializeServerWelcome(++snapshotSequence_, simulation_.player().playerId));
            std::cout << "client connected from " << clientEndpoint_->host << ':' << clientEndpoint_->port << '\n';
            continue;
        }

        if (*type == PacketType::Disconnect && isCurrentClient(packet->from)) {
            std::cout << "client disconnected from " << packet->from.host << ':' << packet->from.port << '\n';
            clientEndpoint_.reset();
            continue;
        }

        if (*type == PacketType::ClientInput) {
            if (!clientEndpoint_) {
                clientEndpoint_ = packet->from;
                std::cout << "client connected from " << clientEndpoint_->host << ':' << clientEndpoint_->port << '\n';
            }
            if (isCurrentClient(packet->from)) {
                lastClientPacketTime_ = std::chrono::steady_clock::now();
            }
            if (isCurrentClient(packet->from)) {
                const auto input = deserializeClientInput(packet->bytes);
                if (!input) {
                    continue;
                }
                simulation_.applyInput(*input);
            }
        }
    }
}

void ServerApplication::checkClientTimeout(std::chrono::steady_clock::time_point now) {
    if (!clientEndpoint_) {
        return;
    }

    if (now - lastClientPacketTime_ >= std::chrono::seconds(5)) {
        std::cout << "client timed out from " << clientEndpoint_->host << ':' << clientEndpoint_->port << '\n';
        clientEndpoint_.reset();
    }
}

void ServerApplication::sendSnapshot() {
    if (!clientEndpoint_) {
        return;
    }

    ServerSnapshot snapshot;
    snapshot.sequence = ++snapshotSequence_;
    snapshot.serverTick = simulation_.tickCount();
    snapshot.player = simulation_.player();
    transport_.sendTo(*clientEndpoint_, serializeServerSnapshot(snapshot));
}

bool ServerApplication::isCurrentClient(const NetworkEndpoint& endpoint) const {
    return clientEndpoint_ && clientEndpoint_->host == endpoint.host && clientEndpoint_->port == endpoint.port;
}

} // namespace game
