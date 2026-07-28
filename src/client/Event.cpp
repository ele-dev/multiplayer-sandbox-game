#include "client/Event.hpp"

#include <SDL3/SDL_events.h>

#include <algorithm>
#include <cstring>

namespace game {

std::optional<Event> convertSdlEvent(const SDL_Event& sdlEvent) {
    Event event{};

    switch (sdlEvent.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        event.type = EventType::Quit;
        return event;
    case SDL_EVENT_KEY_DOWN:
        event.type = EventType::KeyDown;
        event.key.scancode = sdlEvent.key.scancode;
        return event;
    case SDL_EVENT_KEY_UP:
        event.type = EventType::KeyUp;
        event.key.scancode = sdlEvent.key.scancode;
        return event;
    case SDL_EVENT_MOUSE_MOTION:
        event.type = EventType::MouseMove;
        event.mouseMove.x = sdlEvent.motion.x;
        event.mouseMove.y = sdlEvent.motion.y;
        event.mouseMove.xrel = sdlEvent.motion.xrel;
        event.mouseMove.yrel = sdlEvent.motion.yrel;
        return event;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        event.type = EventType::MouseButtonDown;
        event.mouseButton.button = sdlEvent.button.button;
        event.mouseButton.x = sdlEvent.button.x;
        event.mouseButton.y = sdlEvent.button.y;
        return event;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event.type = EventType::MouseButtonUp;
        event.mouseButton.button = sdlEvent.button.button;
        event.mouseButton.x = sdlEvent.button.x;
        event.mouseButton.y = sdlEvent.button.y;
        return event;
    case SDL_EVENT_MOUSE_WHEEL:
        event.type = EventType::MouseWheel;
        event.mouseWheel.x = sdlEvent.wheel.x;
        event.mouseWheel.y = sdlEvent.wheel.y;
        return event;
    case SDL_EVENT_TEXT_INPUT:
        event.type = EventType::TextInput;
        std::copy_n(
            sdlEvent.text.text,
            std::min(std::strlen(sdlEvent.text.text), event.textInput.text.size() - 1),
            event.textInput.text.data()
        );
        return event;
    case SDL_EVENT_WINDOW_RESIZED:
        event.type = EventType::WindowResized;
        event.windowResize.width = sdlEvent.window.data1;
        event.windowResize.height = sdlEvent.window.data2;
        return event;
    default:
        return std::nullopt;
    }
}

} // namespace game
