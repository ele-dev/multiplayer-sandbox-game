#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

namespace game {

class DearImGuiContext {
public:
    DearImGuiContext() = default;
    ~DearImGuiContext();

    DearImGuiContext(const DearImGuiContext&) = delete;
    DearImGuiContext& operator=(const DearImGuiContext&) = delete;

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
