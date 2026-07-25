#include "client/ClientApplication.hpp"
#include "client/ConnectLayer.hpp"
#include "client/DebugOverlayLayer.hpp"
#include "client/HudLayer.hpp"
#include "client/MainMenuLayer.hpp"
#include "client/PauseMenuLayer.hpp"
#include "client/ViewportLayer.hpp"

#include "net/Serialization.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>

#include <iostream>
#include <cstdio>

namespace game {

ClientApplication::~ClientApplication() {
    shutdown();
}

int ClientApplication::run() {
    if (!initialize()) {
        return 1;
    }

    layerStack_.pushLayer(std::make_unique<MainMenuLayer>(
        [this]() { requestPlay(); },
        [this]() { running_ = false; }
    ));

    while (running_) {
        input_.beginFrame();
        processEvents();

        if (input_.quitRequested()) {
            running_ = false;
            break;
        }

        renderer_.clear();
        layerStack_.onUpdate();

        guiLayer_.beginFrame();
        layerStack_.onRender();
        guiLayer_.endFrame();

        updateRelativeMouse();

        SDL_GL_SwapWindow(window_);
        SDL_Delay(1);
    }

    return 0;
}

bool ClientApplication::initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return false;
    }

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

    if (!guiLayer_.initialize(window_, glContext_)) {
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
    guiLayer_.shutdown();
    renderer_.shutdown();
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

Event ClientApplication::convertEvent(const SDL_Event& sdlEvent) const {
    Event event{};

    switch (sdlEvent.type) {
    case SDL_EVENT_QUIT:
        event.type = EventType::Quit;
        break;
    case SDL_EVENT_KEY_DOWN:
        event.type = EventType::KeyDown;
        event.key.scancode = sdlEvent.key.scancode;
        break;
    case SDL_EVENT_KEY_UP:
        event.type = EventType::KeyUp;
        event.key.scancode = sdlEvent.key.scancode;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        event.type = EventType::MouseMove;
        event.mouseMove.xrel = sdlEvent.motion.xrel;
        event.mouseMove.yrel = sdlEvent.motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        event.type = EventType::MouseButtonDown;
        event.mouseButton.button = sdlEvent.button.button;
        event.mouseButton.x = sdlEvent.button.x;
        event.mouseButton.y = sdlEvent.button.y;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event.type = EventType::MouseButtonUp;
        event.mouseButton.button = sdlEvent.button.button;
        event.mouseButton.x = sdlEvent.button.x;
        event.mouseButton.y = sdlEvent.button.y;
        break;
    case SDL_EVENT_WINDOW_RESIZED:
        event.type = EventType::WindowResized;
        event.windowResize.width = sdlEvent.window.data1;
        event.windowResize.height = sdlEvent.window.data2;
        break;
    default:
        break;
    }

    return event;
}

void ClientApplication::processEvents() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        printf("[processEvents] SDL event type: %d (0x%x)\n", sdlEvent.type, sdlEvent.type);
        if (sdlEvent.type == SDL_EVENT_KEY_DOWN && sdlEvent.key.scancode == SDL_SCANCODE_ESCAPE)
            printf("  -> ESC key down detected\n");
        if (sdlEvent.type == SDL_EVENT_QUIT)
            printf("  -> SDL_EVENT_QUIT detected!\n");

        guiLayer_.processEvent(sdlEvent);

        Event event = convertEvent(sdlEvent);

        if (event.type == EventType::WindowResized) {
            renderer_.setViewport(event.windowResize.width, event.windowResize.height);
            layerStack_.resize(event.windowResize.width, event.windowResize.height);
        }

        layerStack_.onEvent(event);

        input_.handleEvent(sdlEvent);
    }
}

void ClientApplication::updateRelativeMouse() {
    const bool wanted = layerStack_.wantsRelativeMouse();
    SDL_SetWindowRelativeMouseMode(window_, wanted);
}

void ClientApplication::requestPlay() {
    while (!layerStack_.empty()) {
        layerStack_.popLayer();
    }
    layerStack_.pushLayer(std::make_unique<ConnectLayer>(
        [this](const std::string& ip) { requestConnect(ip); },
        [this]() { requestReturnToStart(); }
    ));
}

void ClientApplication::requestConnect(const std::string& ip) {
    serverEndpoint_ = {ip, defaultServerPort};
    if (!transport_.open(0)) {
        std::cerr << "Failed to open client UDP socket\n";
        return;
    }
    transportOpen_ = true;

    transport_.sendTo(serverEndpoint_, serializeClientHello(1));
    std::cout << "game_client sending to " << serverEndpoint_.host << ':' << serverEndpoint_.port << '\n';

    isPaused_ = false;

    while (!layerStack_.empty()) {
        layerStack_.popLayer();
    }

    layerStack_.pushLayer(std::make_unique<ViewportLayer>(
        input_,
        camera_,
        debugState_,
        renderer_,
        transport_,
        serverEndpoint_,
        isPaused_,
        [this](bool paused) { onPauseToggled(paused); },
        [this]() { requestReturnToStart(); }
    ));

    layerStack_.pushOverlay(std::make_unique<HudLayer>(isPaused_));
    layerStack_.pushOverlay(std::make_unique<DebugOverlayLayer>(debugState_));
}

void ClientApplication::onPauseToggled(bool paused) {
    isPaused_ = paused;
    if (paused) {
        layerStack_.pushOverlay(std::make_unique<PauseMenuLayer>(
            [this]() { onPauseToggled(false); },
            [this]() { requestReturnToStart(); }
        ));
    } else {
        layerStack_.popLayer();
    }
}

void ClientApplication::requestReturnToStart() {
    if (transportOpen_) {
        transport_.close();
        transportOpen_ = false;
    }
    debugState_ = {};
    isPaused_ = false;

    while (!layerStack_.empty()) {
        layerStack_.popLayer();
    }

    layerStack_.pushLayer(std::make_unique<MainMenuLayer>(
        [this]() { requestPlay(); },
        [this]() { running_ = false; }
    ));
}

} // namespace game
