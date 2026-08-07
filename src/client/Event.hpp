#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>

#include <array>
#include <optional>

namespace game {

enum class EventType {
    KeyDown,
    KeyUp,
    MouseMove,
    MouseButtonDown,
    MouseButtonUp,
    MouseWheel,
    TextInput,
    WindowResized,
    Quit
};

struct Event {
    EventType type;
    bool consumed = false;

    union {
        struct { SDL_Scancode scancode; } key;
        struct { int button; float x; float y; } mouseButton;
        struct { float x; float y; float xrel; float yrel; } mouseMove;
        struct { float x; float y; } mouseWheel;
        struct { std::array<char, 32> text; } textInput;
        struct { int width; int height; } windowResize;
    };
};

std::optional<Event> convertSdlEvent(const SDL_Event& sdlEvent);

} // namespace game
