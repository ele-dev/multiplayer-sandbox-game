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

void LayerStack::requestPushLayer(std::unique_ptr<Layer> layer) {
    pendingOperations_.push_back({PendingOperationType::PushLayer, std::move(layer)});
}

void LayerStack::requestPushOverlay(std::unique_ptr<Layer> layer) {
    pendingOperations_.push_back({PendingOperationType::PushOverlay, std::move(layer)});
}

void LayerStack::requestPopLayer() {
    pendingOperations_.push_back({PendingOperationType::PopLayer, nullptr});
}

void LayerStack::requestClear() {
    pendingOperations_.push_back({PendingOperationType::Clear, nullptr});
}

void LayerStack::applyPendingChanges() {
    while (!pendingOperations_.empty()) {
        auto operations = std::move(pendingOperations_);
        pendingOperations_.clear();

        for (auto& operation : operations) {
            switch (operation.type) {
            case PendingOperationType::PushLayer:
                pushLayer(std::move(operation.layer));
                break;
            case PendingOperationType::PushOverlay:
                pushOverlay(std::move(operation.layer));
                break;
            case PendingOperationType::PopLayer:
                popLayer();
                break;
            case PendingOperationType::Clear:
                while (!layers_.empty()) {
                    popLayer();
                }
                break;
            }
        }
    }
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

        if (event.consumed || layer->isBlocking()) {
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
