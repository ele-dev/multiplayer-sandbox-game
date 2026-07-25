#pragma once

#include "shared/Protocol.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

#include <cstdint>

namespace game {

struct RenderDebugState {
    bool connected = false;
    std::uint32_t snapshotSequence = 0;
    std::uint64_t serverTick = 0;
    PlayerState player = {};
};

class GuiLayer {
public:
    GuiLayer() = default;
    ~GuiLayer();

    GuiLayer(const GuiLayer&) = delete;
    GuiLayer& operator=(const GuiLayer&) = delete;

    bool initialize(SDL_Window* window, SDL_GLContext glContext);
    void processEvent(const SDL_Event& event);
    void beginFrame();
    void endFrame();
    void shutdown();

    bool isInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
};

} // namespace game
