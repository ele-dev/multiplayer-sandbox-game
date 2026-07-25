#pragma once

#include "client/GuiLayer.hpp"
#include "client/Layer.hpp"

namespace game {

class DebugOverlayLayer : public Layer {
public:
    explicit DebugOverlayLayer(const RenderDebugState& debugState)
        : debugState_(debugState)
    {
        setBlocking(false);
    }

    void onRender() override;

private:
    const RenderDebugState& debugState_;
};

} // namespace game
