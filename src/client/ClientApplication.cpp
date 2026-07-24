#include "client/ClientApplication.hpp"

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

    transport_.sendTo(serverEndpoint_, serializeClientHello(++inputSequence_));

    while (running_) {
        input_.beginFrame();
        processEvents();
        sendInput();
        processNetwork();
        renderer_.render(camera_, debugState_);
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
    SDL_SetWindowRelativeMouseMode(window_, true);
    renderer_.setViewport(1280, 720);

    if (!transport_.open(0)) {
        std::cerr << "Failed to open client UDP socket\n";
        return false;
    }
    transportOpen_ = true;

    std::cout << "game_client sending to " << serverEndpoint_.host << ':' << serverEndpoint_.port << '\n';
    return true;
}

void ClientApplication::shutdown() {
    sendDisconnect();
    transport_.close();
    transportOpen_ = false;
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
        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            renderer_.setViewport(event.window.data1, event.window.data2);
        }
        input_.handleEvent(event);
    }

    if (input_.quitRequested()) {
        running_ = false;
    }
}

void ClientApplication::processNetwork() {
    while (auto packet = transport_.receive()) {
        const auto type = readPacketType(packet->bytes);
        if (!type) {
            continue;
        }

        if (*type == PacketType::ServerWelcome) {
            debugState_.connected = true;
            std::cout << "connected to server\n";
            continue;
        }

        if (*type == PacketType::ServerSnapshot) {
            if (auto snapshot = deserializeServerSnapshot(packet->bytes)) {
                camera_.setFromPlayer(snapshot->player);
                debugState_.connected = true;
                debugState_.snapshotSequence = snapshot->sequence;
                debugState_.serverTick = snapshot->serverTick;
                debugState_.player = snapshot->player;
            }
        }
    }
}

void ClientApplication::sendInput() {
    const auto command = input_.command(++inputSequence_, ++clientTick_);
    transport_.sendTo(serverEndpoint_, serializeClientInput(command));
}

void ClientApplication::sendDisconnect() {
    if (!transportOpen_ || disconnectSent_) {
        return;
    }
    transport_.sendTo(serverEndpoint_, serializeDisconnect(++inputSequence_));
    disconnectSent_ = true;
}

} // namespace game
