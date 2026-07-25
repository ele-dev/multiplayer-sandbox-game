#pragma once

#include "client/Scene.hpp"

#include <memory>

namespace game {

class SceneManager {
public:
    void pushScene(std::unique_ptr<Scene> scene);

    void applyPending();

    Scene* current();
    const Scene* current() const;

private:
    std::unique_ptr<Scene> current_;
    std::unique_ptr<Scene> pending_;
};

} // namespace game
