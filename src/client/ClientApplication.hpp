#pragma once

#include "client/Camera.hpp"
#include "client/Input.hpp"
#include "client/OpenGLRenderer.hpp"
#include "net/UdpTransport.hpp"

#include <SDL3/SDL_video.h>

#include <cstdint>

namespace game {

class ClientApplication {
public:
    ClientApplication() = default;
    ~ClientApplication();

    ClientApplication(const ClientApplication&) = delete;
    ClientApplication& operator=(const ClientApplication&) = delete;

    int run();

private:
    bool initialize();
    void shutdown();
    void processEvents();
    void processNetwork();
    void sendInput();
    void sendDisconnect();

    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    Input input_;
    Camera camera_;
    RenderDebugState debugState_;
    OpenGLRenderer renderer_;
    UdpTransport transport_;
    NetworkEndpoint serverEndpoint_ = {"127.0.0.1", defaultServerPort};
    bool running_ = true;
    bool transportOpen_ = false;
    bool disconnectSent_ = false;
    std::uint32_t inputSequence_ = 0;
    std::uint64_t clientTick_ = 0;
};

} // namespace game
