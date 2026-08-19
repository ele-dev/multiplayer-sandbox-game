#include "net/GameNetworkingSocketsTransport.hpp"

#include <steam/isteamnetworkingutils.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

namespace game {

namespace {

std::mutex gnsInitMutex;
int gnsInitRefCount = 0;

bool initializeGameNetworkingSockets() {
    std::lock_guard lock(gnsInitMutex);
    if (gnsInitRefCount > 0) {
        ++gnsInitRefCount;
        return true;
    }

    SteamDatagramErrMsg errorMessage{};
    if (!GameNetworkingSockets_Init(nullptr, errorMessage)) {
        std::cerr << "GameNetworkingSockets_Init failed: " << errorMessage << '\n';
        return false;
    }

    ++gnsInitRefCount;
    return true;
}

void shutdownGameNetworkingSockets() {
    std::lock_guard lock(gnsInitMutex);
    if (gnsInitRefCount <= 0) {
        return;
    }

    --gnsInitRefCount;
    if (gnsInitRefCount == 0) {
        GameNetworkingSockets_Kill();
    }
}

std::atomic<GameNetworkingSocketsTransport*> activeTransport = nullptr;

} // namespace

GameNetworkingSocketsTransport::GameNetworkingSocketsTransport() = default;

GameNetworkingSocketsTransport::~GameNetworkingSocketsTransport() {
    close();
}

bool GameNetworkingSocketsTransport::listen(std::uint16_t port) {
    close();

    if (!initializeGameNetworkingSockets()) {
        return false;
    }

    initialized_ = true;
    interface_ = SteamNetworkingSockets();
    activeTransport.store(this);

    SteamNetworkingIPAddr address;
    address.Clear();
    address.m_port = port;

    SteamNetworkingConfigValue_t option;
    option.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, reinterpret_cast<void*>(connectionStatusChangedCallback));
    listenSocket_ = interface_->CreateListenSocketIP(address, 1, &option);
    if (listenSocket_ == k_HSteamListenSocket_Invalid) {
        std::cerr << "GameNetworkingSockets failed to listen on port " << port << '\n';
        close();
        return false;
    }

    pollGroup_ = interface_->CreatePollGroup();
    if (pollGroup_ == k_HSteamNetPollGroup_Invalid) {
        std::cerr << "GameNetworkingSockets failed to create server poll group\n";
        close();
        return false;
    }

    mode_ = Mode::Server;
    return true;
}

bool GameNetworkingSocketsTransport::connect(const NetworkEndpoint& endpoint) {
    close();

    if (!initializeGameNetworkingSockets()) {
        return false;
    }

    initialized_ = true;
    interface_ = SteamNetworkingSockets();
    activeTransport.store(this);

    SteamNetworkingIPAddr address;
    const std::string addressText = endpoint.host + ':' + std::to_string(endpoint.port);
    if (!address.ParseString(addressText.c_str())) {
        std::cerr << "GameNetworkingSockets invalid server address: " << addressText << '\n';
        close();
        return false;
    }

    SteamNetworkingConfigValue_t option;
    option.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, reinterpret_cast<void*>(connectionStatusChangedCallback));
    connection_ = interface_->ConnectByIPAddress(address, 1, &option);
    if (connection_ == k_HSteamNetConnection_Invalid) {
        std::cerr << "GameNetworkingSockets failed to create client connection\n";
        close();
        return false;
    }

    mode_ = Mode::Client;
    state_ = ConnectionState::Connecting;
    return true;
}

void GameNetworkingSocketsTransport::close() {
    if (interface_ != nullptr) {
        resetConnection();

        if (listenSocket_ != k_HSteamListenSocket_Invalid) {
            interface_->CloseListenSocket(listenSocket_);
            listenSocket_ = k_HSteamListenSocket_Invalid;
        }

        if (pollGroup_ != k_HSteamNetPollGroup_Invalid) {
            interface_->DestroyPollGroup(pollGroup_);
            pollGroup_ = k_HSteamNetPollGroup_Invalid;
        }
    }

    if (activeTransport.load() == this) {
        activeTransport.store(nullptr);
    }

    interface_ = nullptr;
    mode_ = Mode::None;
    state_ = ConnectionState::None;
    failureReason_ = ConnectionFailureReason::None;

    if (initialized_) {
        initialized_ = false;
        shutdownGameNetworkingSockets();
    }
}

bool GameNetworkingSocketsTransport::send(const std::vector<std::uint8_t>& bytes, NetworkSendMode mode) {
    if (interface_ == nullptr || connection_ == k_HSteamNetConnection_Invalid || bytes.empty()) {
        return false;
    }

    const EResult result = interface_->SendMessageToConnection(
        connection_,
        bytes.data(),
        static_cast<uint32>(bytes.size()),
        sendFlags(mode),
        nullptr
    );
    return result == k_EResultOK;
}

void GameNetworkingSocketsTransport::updateConnectionState() {
    if (interface_ != nullptr) {
        interface_->RunCallbacks();
    }
}

std::optional<NetworkPacket> GameNetworkingSocketsTransport::receive() {
    if (interface_ == nullptr) {
        return std::nullopt;
    }

    interface_->RunCallbacks();

    ISteamNetworkingMessage* message = nullptr;
    int messageCount = 0;
    if (mode_ == Mode::Server) {
        if (pollGroup_ == k_HSteamNetPollGroup_Invalid) {
            return std::nullopt;
        }
        messageCount = interface_->ReceiveMessagesOnPollGroup(pollGroup_, &message, 1);
    } else if (mode_ == Mode::Client) {
        if (connection_ == k_HSteamNetConnection_Invalid) {
            return std::nullopt;
        }
        messageCount = interface_->ReceiveMessagesOnConnection(connection_, &message, 1);
    }

    if (messageCount <= 0 || message == nullptr) {
        return std::nullopt;
    }

    NetworkPacket packet;
    packet.from = endpointForConnection(message->m_conn);
    const auto* data = static_cast<const std::uint8_t*>(message->m_pData);
    packet.bytes.assign(data, data + message->m_cbSize);
    message->Release();
    return packet;
}

void GameNetworkingSocketsTransport::connectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* info) {
    if (auto* transport = activeTransport.load()) {
        transport->onConnectionStatusChanged(info);
    }
}

void GameNetworkingSocketsTransport::onConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) {
    switch (info->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting:
        if (mode_ != Mode::Server || interface_ == nullptr) {
            return;
        }
        if (connection_ != k_HSteamNetConnection_Invalid) {
            interface_->CloseConnection(info->m_hConn, 0, "Server already has a client", false);
            return;
        }
        if (interface_->AcceptConnection(info->m_hConn) != k_EResultOK) {
            interface_->CloseConnection(info->m_hConn, 0, "AcceptConnection failed", false);
            return;
        }
        if (!interface_->SetConnectionPollGroup(info->m_hConn, pollGroup_)) {
            interface_->CloseConnection(info->m_hConn, 0, "SetConnectionPollGroup failed", false);
            return;
        }
        connection_ = info->m_hConn;
        state_ = ConnectionState::Connected;
        std::cout << "GameNetworkingSockets client connected: " << info->m_info.m_szConnectionDescription << '\n';
        break;
    case k_ESteamNetworkingConnectionState_Connected:
        if (mode_ == Mode::Client && info->m_hConn == connection_) {
            state_ = ConnectionState::Connected;
            std::cout << "GameNetworkingSockets connected to server\n";
        }
        break;
    case k_ESteamNetworkingConnectionState_ClosedByPeer:
        if (info->m_hConn == connection_) {
            failureReason_ = ConnectionFailureReason::ClosedByPeer;
            state_ = ConnectionState::Failed;
            std::cout << "GameNetworkingSockets connection closed by peer: " << info->m_info.m_szEndDebug << '\n';
            resetConnection();
        } else if (interface_ != nullptr) {
            interface_->CloseConnection(info->m_hConn, 0, nullptr, false);
        }
        break;
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        if (info->m_hConn == connection_) {
            failureReason_ = ConnectionFailureReason::ProblemDetectedLocally;
            state_ = ConnectionState::Failed;
            std::cout << "GameNetworkingSockets problem detected locally: " << info->m_info.m_szEndDebug << '\n';
            resetConnection();
        } else if (interface_ != nullptr) {
            interface_->CloseConnection(info->m_hConn, 0, nullptr, false);
        }
        break;
    case k_ESteamNetworkingConnectionState_None:
    case k_ESteamNetworkingConnectionState_FindingRoute:
    case k_ESteamNetworkingConnectionState_FinWait:
    case k_ESteamNetworkingConnectionState_Linger:
    case k_ESteamNetworkingConnectionState_Dead:
    default:
        break;
    }
}

NetworkEndpoint GameNetworkingSocketsTransport::endpointForConnection(HSteamNetConnection connection) const {
    if (interface_ == nullptr || connection == k_HSteamNetConnection_Invalid) {
        return {};
    }

    SteamNetConnectionInfo_t info;
    if (!interface_->GetConnectionInfo(connection, &info)) {
        return {};
    }

    char address[SteamNetworkingIPAddr::k_cchMaxString] = {};
    info.m_addrRemote.ToString(address, sizeof(address), false);
    return {address, info.m_addrRemote.m_port};
}

int GameNetworkingSocketsTransport::sendFlags(NetworkSendMode mode) const {
    return mode == NetworkSendMode::Reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
}

GameNetworkingSocketsTransport::ConnectionState GameNetworkingSocketsTransport::getConnectionState() const {
    return state_;
}

GameNetworkingSocketsTransport::ConnectionFailureReason GameNetworkingSocketsTransport::getFailureReason() const {
    return failureReason_;
}

bool GameNetworkingSocketsTransport::isConnected() const {
    return state_ == ConnectionState::Connected;
}

void GameNetworkingSocketsTransport::resetConnection() {
    if (interface_ != nullptr && connection_ != k_HSteamNetConnection_Invalid) {
        interface_->CloseConnection(connection_, 0, nullptr, false);
    }
    connection_ = k_HSteamNetConnection_Invalid;
    state_ = ConnectionState::None;
    failureReason_ = ConnectionFailureReason::None;
}

} // namespace game
