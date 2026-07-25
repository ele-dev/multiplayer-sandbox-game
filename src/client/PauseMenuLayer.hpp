#pragma once

#include "client/Layer.hpp"

#include <functional>

namespace game {

class PauseMenuLayer : public Layer {
public:
    using ResumeCallback = std::function<void()>;
    using ReturnToStartCallback = std::function<void()>;

    PauseMenuLayer(ResumeCallback onResume, ReturnToStartCallback onReturnToStart);

    void onRender() override;

private:
    ResumeCallback onResume_;
    ReturnToStartCallback onReturnToStart_;
};

} // namespace game
