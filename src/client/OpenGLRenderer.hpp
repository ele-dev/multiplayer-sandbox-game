#pragma once

#include "client/Camera.hpp"
#include "client/graphics/DebugLineRenderer.hpp"
#include "client/graphics/SolidMeshRenderer.hpp"

#include <glm/vec3.hpp>

namespace game {

class OpenGLRenderer {
public:
    ~OpenGLRenderer();

    void clear();
    void setViewport(int width, int height);
    void setObjectColor(glm::vec3 color);
    void render(const Camera& camera);
    void shutdown();

private:
    bool initialize();

    int width_ = 1280;
    int height_ = 720;
    bool initialized_ = false;
    glm::vec3 objectColor_ = {0.20f, 0.70f, 1.00f};
    SolidMeshRenderer solidMeshRenderer_;
    DebugLineRenderer debugLineRenderer_;
};

} // namespace game
