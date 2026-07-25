#pragma once

#include "client/Camera.hpp"

namespace game {

class OpenGLRenderer {
public:
    ~OpenGLRenderer();

    void clear();
    void setViewport(int width, int height);
    void render(const Camera& camera);
    void shutdown();

private:
    bool initialize();

    int width_ = 1280;
    int height_ = 720;
    unsigned int program_ = 0;
    unsigned int vertexArray_ = 0;
    unsigned int vertexBuffer_ = 0;
};

} // namespace game
