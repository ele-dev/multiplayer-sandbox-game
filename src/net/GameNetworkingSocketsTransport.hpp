#pragma once

#include "net/NetworkTransport.hpp"

#include <steam/steamnetworkingsockets.h>

#include <cstdint>

namespace game {

class GameNetworkingSocketsTransport final : public NetworkTransport {
public:
    GameNetworkingSocketsTransport();
    ~GameNetworkingSocketsTransport() override;

    GameNetworkingSocketsTransport(const GameNetworkingSocketsTransport&) = delete;
    GameNetworkingSocketsTransport& operator=(const GameNetworkingSocketsTransport&) = delete;

    bool listen(std::uint16_t port) override;
    bool connect(const NetworkEndpoint& endpoint) override;
    void close() override;
    bool send(const std::vector<std::uint8_t>& bytes, NetworkSendMode mode = NetworkSendMode::Unreliable) override;
    std::optional<NetworkPacket> receive() override;

    enum class ConnectionState {
        None,
        Connecting,
        Connected,
        Failed
    };

    enum class ConnectionFailureReason {
        None,
        Timeout,
        ClosedByPeer,
        ProblemDetectedLocally
    };

    [[nodiscard]] ConnectionState getConnectionState() const;
    [[nodiscard]] ConnectionFailureReason getFailureReason() const;
    [[nodiscard]] bool isConnected() const;

private:
    enum class Mode {
        None,
        Server,
        Client
    };

    static void connectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info);
    void onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);

    [[nodiscard]] NetworkEndpoint endpointForConnection(HSteamNetConnection connection) const;
    [[nodiscard]] int sendFlags(NetworkSendMode mode) const;
    void resetConnection();

    ISteamNetworkingSockets* interface_ = nullptr;
    HSteamListenSocket listenSocket_ = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup pollGroup_ = k_HSteamNetPollGroup_Invalid;
    HSteamNetConnection connection_ = k_HSteamNetConnection_Invalid;
    Mode mode_ = Mode::None;
    bool initialized_ = false;
    ConnectionState state_ = ConnectionState::None;
    ConnectionFailureReason failureReason_ = ConnectionFailureReason::None;
};

} // namespace game
