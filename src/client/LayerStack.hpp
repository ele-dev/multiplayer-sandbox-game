#pragma once

#include "client/Layer.hpp"

#include <memory>
#include <vector>

namespace game {

class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    void pushLayer(std::unique_ptr<Layer> layer);
    void pushOverlay(std::unique_ptr<Layer> layer);
    std::unique_ptr<Layer> popLayer();

    void onUpdate();
    void onRender();
    void onEvent(Event& event);

    void resize(int width, int height);

    bool wantsRelativeMouse() const;

    int windowWidth() const { return windowWidth_; }
    int windowHeight() const { return windowHeight_; }

    std::size_t size() const { return layers_.size(); }
    bool empty() const { return layers_.empty(); }

    auto begin() { return layers_.begin(); }
    auto end() { return layers_.end(); }
    auto begin() const { return layers_.begin(); }
    auto end() const { return layers_.end(); }

private:
    std::vector<std::unique_ptr<Layer>> layers_;
    std::size_t nextInsertIndex_ = 0;
    int windowWidth_ = 1280;
    int windowHeight_ = 720;
};

} // namespace game
