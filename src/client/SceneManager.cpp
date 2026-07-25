#include "client/SceneManager.hpp"

namespace game {

void SceneManager::pushScene(std::unique_ptr<Scene> scene) {
    pending_ = std::move(scene);
}

void SceneManager::applyPending() {
    if (pending_) {
        if (current_) {
            current_->onExit();
        }
        current_ = std::move(pending_);
        current_->onEnter();
    }
}

Scene* SceneManager::current() {
    return current_.get();
}

const Scene* SceneManager::current() const {
    return current_.get();
}

} // namespace game
