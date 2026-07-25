#pragma once

#include "client/Layer.hpp"

#include <functional>

namespace game {

class MainMenuLayer : public Layer {
public:
    using PlayCallback = std::function<void()>;
    using ExitCallback = std::function<void()>;

    MainMenuLayer(PlayCallback onPlay, ExitCallback onExit);

    void onRender() override;

private:
    PlayCallback onPlay_;
    ExitCallback onExit_;
};

} // namespace game
