#pragma once

#include "client/Scene.hpp"

#include <functional>

namespace game {

class StartScene : public Scene {
public:
    using PlayCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    StartScene(PlayCallback onPlay, ExitCallback onExit);

    void renderImGui() override;

private:
    PlayCallback onPlay_;
    ExitCallback onExit_;
};

} // namespace game
