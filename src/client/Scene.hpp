#pragma once

#include <SDL3/SDL_events.h>

namespace game {

class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void handleEvent(const SDL_Event& /*event*/) {}
    virtual void update() {}

    virtual void renderImGui() {}

    virtual bool wantsRelativeMouse() const { return false; }
    virtual bool wantsRender3D() const { return false; }
};

} // namespace game
