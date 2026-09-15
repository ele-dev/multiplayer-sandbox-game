#pragma once

#include "client/Layer.hpp"

#include <functional>

namespace game {

class PauseMenuLayer : public Layer {
public:
    using ResumeCallback = std::function<void()>;
    using ReturnToStartCallback = std::function<void()>;
    using SettingsCallback = std::function<void()>;

    PauseMenuLayer(ResumeCallback onResume, ReturnToStartCallback onReturnToStart, SettingsCallback onSettings);

    void onEvent(Event& event) override;
    void onRender() override;

private:
    ResumeCallback onResume_;
    ReturnToStartCallback onReturnToStart_;
    SettingsCallback settingsCallback_;
};

} // namespace game
