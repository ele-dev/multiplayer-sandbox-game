#include "client/ClientApplication.hpp"
#include "client/ConnectScene.hpp"
#include "client/InGameScene.hpp"
#include "client/StartScene.hpp"

#include "net/Serialization.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>

#include <iostream>

namespace game {

ClientApplication::~ClientApplication() {
    shutdown();
}

int ClientApplication::run() {
    if (!initialize()) {
        return 1;
    }

    sceneManager_.pushScene(std::make_unique<StartScene>(
        [this]() { requestPlay(); },
        [this]() { running_ = false; }
    ));

    while (running_) {
        input_.beginFrame();
        processEvents();
        sceneManager_.applyPending();

        if (input_.quitRequested()) {
            running_ = false;
            break;
        }

        if (auto* scene = sceneManager_.current()) {
            scene->update();

            renderer_.render(camera_);

            guiLayer_.beginFrame();
            scene->renderImGui();
            guiLayer_.endFrame();

            updateRelativeMouse();
        }

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

void ClientApplication::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        guiLayer_.processEvent(event);
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            renderer_.setViewport(event.window.data1, event.window.data2);
        }
        if (auto* scene = sceneManager_.current()) {
            scene->handleEvent(event);
        }
        input_.handleEvent(event);
    }

    if (input_.quitRequested()) {
        running_ = false;
    }
}

void ClientApplication::updateRelativeMouse() {
    const bool wanted = sceneManager_.current() && sceneManager_.current()->wantsRelativeMouse();
    SDL_SetWindowRelativeMouseMode(window_, wanted);
}

void ClientApplication::requestPlay() {
    sceneManager_.pushScene(std::make_unique<ConnectScene>(
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

    inputSequence_ = 0;

    transport_.sendTo(serverEndpoint_, serializeClientHello(1));
    std::cout << "game_client sending to " << serverEndpoint_.host << ':' << serverEndpoint_.port << '\n';

    sceneManager_.pushScene(std::make_unique<InGameScene>(
        input_,
        camera_,
        debugState_,
        transport_,
        serverEndpoint_,
        [this]() { requestReturnToStart(); }
    ));
}

void ClientApplication::requestReturnToStart() {
    if (transportOpen_) {
        transport_.close();
        transportOpen_ = false;
    }
    debugState_ = {};

    sceneManager_.pushScene(std::make_unique<StartScene>(
        [this]() { requestPlay(); },
        [this]() { running_ = false; }
    ));
}

} // namespace game
