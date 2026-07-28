#pragma once

#include "client/Camera.hpp"
#include "client/DearImGuiContext.hpp"
#include "client/LayerStack.hpp"
#include "client/OpenGLRenderer.hpp"
#include "client/RenderDebugState.hpp"
#include "net/UdpTransport.hpp"

#include <SDL3/SDL_video.h>

#include <cstdint>
#include <string>

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
    void updateRelativeMouse();

    void requestPlay();
    void requestConnect(const std::string& ip);
    void requestReturnToStart();
    void onPauseToggled(bool paused);

    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    Camera camera_;
    RenderDebugState debugState_;
    DearImGuiContext dearImGuiContext_;
    OpenGLRenderer renderer_;
    UdpTransport transport_;
    LayerStack layerStack_;
    NetworkEndpoint serverEndpoint_ = {"127.0.0.1", defaultServerPort};
    bool running_ = true;
    bool isPaused_ = false;
    bool transportOpen_ = false;
};

} // namespace game
