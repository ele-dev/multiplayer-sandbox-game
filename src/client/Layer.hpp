#pragma once

#include "client/Event.hpp"

namespace game {

class Layer {
public:
    virtual ~Layer() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}

    virtual void onUpdate() {}
    virtual void onRender() {}
    virtual void onResize(int /*width*/, int /*height*/) {}

    virtual void onEvent(Event& /*event*/) {}

    virtual bool wantsRelativeMouse() const { return false; }

    bool isVisible() const { return visible_; }
    bool isBlocking() const { return blocking_; }

    void setVisible(bool v) { visible_ = v; }
    void setBlocking(bool b) { blocking_ = b; }

private:
    bool visible_ = true;
    bool blocking_ = false;
};

} // namespace game
