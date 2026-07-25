#include "server/ServerApplication.hpp"

#include "net/Serialization.hpp"

#include <glm/glm.hpp>

#include <chrono>
#include <iostream>
#include <thread>

namespace game {

int ServerApplication::run() {
    std::cout << std::unitbuf;

    loadConfig();

    if (!transport_.open(port_)) {
        std::cerr << "Failed to bind UDP server port " << port_ << '\n';
        return 1;
    }

    std::cout << "game_server listening on UDP port " << port_ << '\n';
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

        while (accumulator >= tickSeconds_) {
            simulation_.tick(static_cast<float>(tickSeconds_));
            sendSnapshot();
            accumulator -= tickSeconds_;
        }

        if (now - lastLog >= logInterval_) {
            const auto& player = simulation_.player();
            std::cout << "tick=" << simulation_.tickCount() << " player=(" << player.position.x << ", "
                      << player.position.y << ", " << player.position.z << ")\n";
            lastLog = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ServerApplication::loadConfig() {
    if (!config_.loadFile("server.cfg")) {
        std::cout << "No server.cfg found, using defaults\n";
        return;
    }

    std::cout << "Loaded server.cfg\n";

    port_ = static_cast<std::uint16_t>(config_.getInt("server.port", port_));

    const int tickRate = config_.getInt("server.tick_rate", 60);
    tickSeconds_ = 1.0 / std::max(tickRate, 1);

    const int timeoutSecs = config_.getInt("server.client_timeout_seconds", static_cast<int>(clientTimeout_.count()));
    clientTimeout_ = std::chrono::seconds(std::max(timeoutSecs, 1));

    const int logSecs = config_.getInt("server.log_interval_seconds", static_cast<int>(logInterval_.count()));
    logInterval_ = std::chrono::seconds(std::max(logSecs, 0));

    const float moveSpeed = config_.getFloat("player.move_speed", 4.5f);
    simulation_.setMoveSpeed(moveSpeed);

    const float sensitivity = config_.getFloat("player.mouse_sensitivity", 0.0025f);
    simulation_.setMouseSensitivity(sensitivity);

    const float maxPitchDegrees = config_.getFloat("player.max_pitch_degrees", 85.94f);
    simulation_.setMaxPitchRadians(glm::radians(maxPitchDegrees));

    std::cout << "  server.port=" << port_ << '\n';
    std::cout << "  server.tick_rate=" << tickRate << '\n';
    std::cout << "  server.client_timeout_seconds=" << timeoutSecs << '\n';
    std::cout << "  server.log_interval_seconds=" << logSecs << '\n';
    std::cout << "  player.move_speed=" << moveSpeed << '\n';
    std::cout << "  player.mouse_sensitivity=" << sensitivity << '\n';
    std::cout << "  player.max_pitch_degrees=" << maxPitchDegrees << '\n';
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

    if (now - lastClientPacketTime_ >= clientTimeout_) {
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
