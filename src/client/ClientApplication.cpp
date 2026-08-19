#include "client/ClientApplication.hpp"
#include "client/ConnectingLayer.hpp"
#include "client/DebugOverlayLayer.hpp"
#include "client/ErrorPopupLayer.hpp"
#include "client/HudLayer.hpp"
#include "client/MainMenuLayer.hpp"
#include "client/PauseMenuLayer.hpp"
#include "client/ViewportLayer.hpp"

#include "net/Serialization.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

namespace game {

namespace {

constexpr float frameTimeSmoothingAlpha = 0.08f;
constexpr float maxDisplayedFrameTimeMs = 250.0f;

} // namespace

ClientApplication::~ClientApplication() {
    shutdown();
}

int ClientApplication::run() {
    if (!initialize()) {
        return 1;
    }

    layerStack_.requestPushLayer(std::make_unique<MainMenuLayer>(
        [this](const std::string& ip) { requestConnect(ip); },
        [this]() { running_ = false; }
    ));
    layerStack_.applyPendingChanges();

    while (running_) {
        updateFrameTiming();

        processEvents();
        layerStack_.applyPendingChanges();

        renderer_.clear();
        layerStack_.onUpdate();
        layerStack_.applyPendingChanges();

        dearImGuiContext_.beginFrame();
        layerStack_.onRender();
        dearImGuiContext_.endFrame();
        layerStack_.applyPendingChanges();

        updateRelativeMouse();

        SDL_GL_SwapWindow(window_);
        SDL_Delay(1);
    }

    return 0;
}

bool ClientApplication::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return false;
    }

    openAvailableGamepads();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window_ = SDL_CreateWindow("multiplayer_game client", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window_ == nullptr) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        return false;
    }

    glContext_ = SDL_GL_CreateContext(window_);
    if (glContext_ == nullptr) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
        return false;
    }

    SDL_GL_SetSwapInterval(1);
    renderer_.setViewport(1280, 720);

    if (!dearImGuiContext_.initialize(window_, glContext_)) {
        std::cerr << "Failed to initialize Dear ImGui\n";
        return false;
    }

    std::cout << "game_client initialized\n";
    return true;
}

void ClientApplication::shutdown() {
    if (transportOpen_) {
        transport_.close();
        transportOpen_ = false;
    }
    dearImGuiContext_.shutdown();
    renderer_.shutdown();
    for (SDL_Gamepad* gamepad : gamepads_) {
        SDL_CloseGamepad(gamepad);
    }
    gamepads_.clear();
    if (glContext_ != nullptr) {
        SDL_GL_DestroyContext(glContext_);
        glContext_ = nullptr;
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

void ClientApplication::updateFrameTiming() {
    const auto now = std::chrono::steady_clock::now();
    if (!hasFrameTime_) {
        lastFrameTime_ = now;
        hasFrameTime_ = true;
        return;
    }

    const std::chrono::duration<float, std::milli> elapsed = now - lastFrameTime_;
    lastFrameTime_ = now;

    if (elapsed.count() <= 0.0f || !std::isfinite(elapsed.count())) {
        return;
    }

    const float frameTimeMs = std::min(elapsed.count(), maxDisplayedFrameTimeMs);
    if (smoothedFrameTimeMs_ <= 0.0f) {
        smoothedFrameTimeMs_ = frameTimeMs;
    } else {
        smoothedFrameTimeMs_ += frameTimeSmoothingAlpha * (frameTimeMs - smoothedFrameTimeMs_);
    }

    debugState_.frameTimeMs = smoothedFrameTimeMs_;
    debugState_.framesPerSecond = smoothedFrameTimeMs_ > 0.0f ? 1000.0f / smoothedFrameTimeMs_ : 0.0f;
}

void ClientApplication::processEvents() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        dearImGuiContext_.processEvent(sdlEvent);

        auto event = convertSdlEvent(sdlEvent);
        if (!event) {
            continue;
        }

        if (event->type == EventType::Quit) {
            running_ = false;
            continue;
        }

        if (event->type == EventType::WindowResized) {
            renderer_.setViewport(event->windowResize.width, event->windowResize.height);
            layerStack_.resize(event->windowResize.width, event->windowResize.height);
            continue;
        }

        if (event->type == EventType::GamepadAdded) {
            openGamepad(event->gamepadDevice.id);
        }

        if (event->type == EventType::GamepadRemoved) {
            closeGamepad(event->gamepadDevice.id);
        }

        // Handle additional application-wide events here
        // ...

        layerStack_.onEvent(*event);
    }

    if (hasPendingConnection_) {
        const auto connectionState = transport_.getConnectionState();
        
        if (connectionState == GameNetworkingSocketsTransport::ConnectionState::Connected) {
            hasPendingConnection_ = false;
            connectionStartTime_ = {};

            layerStack_.requestClear();

            layerStack_.requestPushLayer(std::make_unique<ViewportLayer>(
                camera_,
                debugState_,
                renderer_,
                transport_,
                serverEndpoint_,
                isPaused_,
                [this](bool paused) { onPauseToggled(paused); },
                [this]() { requestReturnToStart(); }
            ));

            layerStack_.requestPushOverlay(std::make_unique<HudLayer>(isPaused_));
            layerStack_.requestPushOverlay(std::make_unique<DebugOverlayLayer>(debugState_));
        }
        else if (connectionState == GameNetworkingSocketsTransport::ConnectionState::Failed) {
            hasPendingConnection_ = false;
            connectionStartTime_ = {};

            layerStack_.requestClear();

            layerStack_.requestPushOverlay(std::make_unique<ErrorPopupLayer>([this]() {
                requestReturnToStart();
            }));
        }
        else if (connectionState == GameNetworkingSocketsTransport::ConnectionState::Connecting) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - connectionStartTime_
            ).count();

            if (elapsed >= 5000) {
                std::cerr << "Connection timeout after 5 seconds\n";
                hasPendingConnection_ = false;
                connectionStartTime_ = {};

                layerStack_.requestClear();

                layerStack_.requestPushOverlay(std::make_unique<ErrorPopupLayer>([this]() {
                    requestReturnToStart();
                }));
            }
        }
    }
}

void ClientApplication::updateRelativeMouse() {
    const bool wanted = layerStack_.wantsRelativeMouse();
    SDL_SetWindowRelativeMouseMode(window_, wanted);
}

void ClientApplication::openAvailableGamepads() {
    int gamepadCount = 0;
    SDL_JoystickID* gamepadIds = SDL_GetGamepads(&gamepadCount);
    if (gamepadIds == nullptr) {
        return;
    }

    for (int i = 0; i < gamepadCount; ++i) {
        openGamepad(gamepadIds[i]);
    }

    SDL_free(gamepadIds);
}

void ClientApplication::openGamepad(SDL_JoystickID id) {
    const auto alreadyOpen = std::any_of(gamepads_.begin(), gamepads_.end(), [id](SDL_Gamepad* gamepad) {
        return SDL_GetGamepadID(gamepad) == id;
    });
    if (alreadyOpen) {
        return;
    }

    SDL_Gamepad* gamepad = SDL_OpenGamepad(id);
    if (gamepad == nullptr) {
        std::cerr << "SDL_OpenGamepad failed: " << SDL_GetError() << '\n';
        return;
    }

    gamepads_.push_back(gamepad);
    const char* gamepadName = SDL_GetGamepadName(gamepad);
    std::cout << "gamepad connected: " << (gamepadName != nullptr ? gamepadName : "unknown") << '\n';
}

void ClientApplication::closeGamepad(SDL_JoystickID id) {
    const auto it = std::find_if(gamepads_.begin(), gamepads_.end(), [id](SDL_Gamepad* gamepad) {
        return SDL_GetGamepadID(gamepad) == id;
    });
    if (it == gamepads_.end()) {
        return;
    }

    std::cout << "gamepad disconnected\n";
    SDL_CloseGamepad(*it);
    gamepads_.erase(it);
}

void ClientApplication::requestConnect(const std::string& ip) {
    serverEndpoint_ = {ip, defaultServerPort};
    if (!transport_.connect(serverEndpoint_)) {
        std::cerr << "Failed to connect to server\n";
        return;
    }
    transportOpen_ = true;
    hasPendingConnection_ = true;
    connectionStartTime_ = std::chrono::steady_clock::now();

    transport_.send(serializeClientHello(1), NetworkSendMode::Reliable);
    std::cout << "game_client sending to " << serverEndpoint_.host << ':' << serverEndpoint_.port << '\n';

    isPaused_ = false;

    layerStack_.requestClear();

    layerStack_.requestPushLayer(std::make_unique<ConnectingLayer>(
        serverEndpoint_.host,
        serverEndpoint_.port
    ));
}

void ClientApplication::onPauseToggled(bool paused) {
    isPaused_ = paused;
    if (paused) {
        layerStack_.requestPushOverlay(std::make_unique<PauseMenuLayer>(
            [this]() { onPauseToggled(false); },
            [this]() { requestReturnToStart(); }
        ));
    } else {
        layerStack_.requestPopLayer();
    }
}

void ClientApplication::requestReturnToStart() {
    if (transportOpen_) {
        transportOpen_ = false;
    }
    debugState_ = {};
    isPaused_ = false;

    layerStack_.requestClear();

    layerStack_.requestPushLayer(std::make_unique<MainMenuLayer>(
        [this](const std::string& ip) { requestConnect(ip); },
        [this]() { running_ = false; }
    ));
}

} // namespace game
