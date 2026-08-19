#pragma once

#include "client/Camera.hpp"
#include "client/DearImGuiContext.hpp"
#include "client/LayerStack.hpp"
#include "client/OpenGLRenderer.hpp"
#include "client/RenderDebugState.hpp"
#include "net/GameNetworkingSocketsTransport.hpp"

#include <chrono>

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_video.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

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
    void updateFrameTiming();
    void processEvents();
    void updateRelativeMouse();
    void openAvailableGamepads();
    void openGamepad(SDL_JoystickID id);
    void closeGamepad(SDL_JoystickID id);

    void requestConnect(const std::string& ip);
    void requestReturnToStart();
    void onPauseToggled(bool paused);

    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    std::vector<SDL_Gamepad*> gamepads_;
    Camera camera_;
    RenderDebugState debugState_;
    DearImGuiContext dearImGuiContext_;
    OpenGLRenderer renderer_;
    GameNetworkingSocketsTransport transport_;
    LayerStack layerStack_;
    NetworkEndpoint serverEndpoint_ = {"127.0.0.1", defaultServerPort};
    std::chrono::steady_clock::time_point lastFrameTime_ = {};
    std::chrono::steady_clock::time_point connectionStartTime_ = {};
    bool running_ = true;
    bool isPaused_ = false;
    bool transportOpen_ = false;
    bool hasFrameTime_ = false;
    bool hasPendingConnection_ = false;
    float smoothedFrameTimeMs_ = 0.0f;
};

} // namespace game
