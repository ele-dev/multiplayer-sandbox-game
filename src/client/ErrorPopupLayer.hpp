#pragma once
#include "client/Layer.hpp"
#include <functional>

namespace game {

class ErrorPopupLayer : public Layer {
public:
    using CloseCallback = std::function<void()>;

    ErrorPopupLayer(CloseCallback onClose);

    void onRender() override;

private:
    CloseCallback onClose_;
};

} // namespace game
