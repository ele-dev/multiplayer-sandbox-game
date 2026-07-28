#include "net/UdpTransport.hpp"

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <array>
#include <cstring>
#include <limits>

namespace game {
namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle invalidSocket = INVALID_SOCKET;

bool ensureSocketsStarted() {
    static bool started = [] {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    return started;
}

void closeSocket(SocketHandle socket) {
    closesocket(socket);
}
#else
using SocketHandle = int;
constexpr SocketHandle invalidSocket = -1;

bool ensureSocketsStarted() {
    return true;
}

void closeSocket(SocketHandle socket) {
    close(socket);
}
#endif

bool setNonBlocking(SocketHandle socket) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
    const int flags = fcntl(socket, F_GETFL, 0);
    return flags >= 0 && fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}

} // namespace

UdpTransport::UdpTransport() = default;

UdpTransport::~UdpTransport() {
    close();
}

bool UdpTransport::open(std::uint16_t localPort) {
    close();

    if (!ensureSocketsStarted()) {
        return false;
    }

    const SocketHandle handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (handle == invalidSocket) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(localPort);

    if (bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 || !setNonBlocking(handle)) {
        closeSocket(handle);
        return false;
    }

    socket_ = static_cast<std::intptr_t>(handle);
    return true;
}

void UdpTransport::close() {
    if (socket_ == -1) {
        return;
    }
    closeSocket(static_cast<SocketHandle>(socket_));
    socket_ = -1;
}

bool UdpTransport::sendTo(const NetworkEndpoint& endpoint, const std::vector<std::uint8_t>& bytes) {
    if (socket_ == -1) {
        return false;
    }
    if (bytes.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(endpoint.port);
    if (inet_pton(AF_INET, endpoint.host.c_str(), &address.sin_addr) != 1) {
        return false;
    }

    const int sent = sendto(
        static_cast<SocketHandle>(socket_),
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<int>(bytes.size()),
        0,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    );
    return sent == static_cast<int>(bytes.size());
}

std::optional<NetworkPacket> UdpTransport::receive() {
    if (socket_ == -1) {
        return std::nullopt;
    }

    std::array<std::uint8_t, 1400> buffer{};
    sockaddr_in from{};
#ifdef _WIN32
    int fromLength = sizeof(from);
#else
    socklen_t fromLength = sizeof(from);
#endif
    const int received = recvfrom(
        static_cast<SocketHandle>(socket_),
        reinterpret_cast<char*>(buffer.data()),
        static_cast<int>(buffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&from),
        &fromLength
    );

    if (received <= 0) {
        return std::nullopt;
    }

    char host[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &from.sin_addr, host, sizeof(host));

    NetworkPacket packet;
    packet.from = {host, ntohs(from.sin_port)};
    packet.bytes.assign(buffer.begin(), buffer.begin() + received);
    return packet;
}

} // namespace game
