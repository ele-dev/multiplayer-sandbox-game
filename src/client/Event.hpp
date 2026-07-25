#pragma once

#include <SDL3/SDL_scancode.h>

namespace game {

enum class EventType {
    KeyDown,
    KeyUp,
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    WindowResized,
    Quit
};

struct Event {
    EventType type;
    bool handled = false;

    union {
        struct { SDL_Scancode scancode; } key;
        struct { int button; float x; float y; } mouseButton;
        struct { float xrel; float yrel; } mouseMove;
        struct { int width; int height; } windowResize;
    };
};

} // namespace game
