#include "client/LayerStack.hpp"

namespace game {

LayerStack::~LayerStack() {
    while (!layers_.empty()) {
        layers_.back()->onDetach();
        layers_.pop_back();
    }
}

void LayerStack::pushLayer(std::unique_ptr<Layer> layer) {
    layer->onAttach();
    layers_.insert(layers_.begin() + static_cast<std::ptrdiff_t>(nextInsertIndex_), std::move(layer));
    ++nextInsertIndex_;
}

void LayerStack::pushOverlay(std::unique_ptr<Layer> layer) {
    layer->onAttach();
    layers_.push_back(std::move(layer));
}

std::unique_ptr<Layer> LayerStack::popLayer() {
    if (layers_.empty()) {
        return nullptr;
    }

    auto layer = std::move(layers_.back());
    layers_.pop_back();
    layer->onDetach();

    if (nextInsertIndex_ > layers_.size()) {
        nextInsertIndex_ = layers_.size();
    }

    return layer;
}

void LayerStack::onUpdate() {
    for (auto& layer : layers_) {
        if (layer->isVisible()) {
            layer->onUpdate();
        }
    }
}

void LayerStack::onRender() {
    for (auto& layer : layers_) {
        if (layer->isVisible()) {
            layer->onRender();
        }
    }
}

void LayerStack::onEvent(Event& event) {
    for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
        auto& layer = *it;
        if (!layer->isVisible()) {
            continue;
        }

        layer->onEvent(event);

        if (event.handled && layer->isBlocking()) {
            break;
        }
    }
}

void LayerStack::resize(int width, int height) {
    windowWidth_ = width;
    windowHeight_ = height;
    for (auto& layer : layers_) {
        if (layer->isVisible()) {
            layer->onResize(width, height);
        }
    }
}

bool LayerStack::wantsRelativeMouse() const {
    for (auto it = layers_.rbegin(); it != layers_.rend(); ++it) {
        auto& layer = *it;
        if (!layer->isVisible()) {
            continue;
        }
        if (layer->wantsRelativeMouse()) {
            return true;
        }
    }
    return false;
}

} // namespace game
