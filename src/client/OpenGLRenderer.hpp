#pragma once

#include "client/Camera.hpp"

#include "shared/Protocol.hpp"

#include <cstdint>

namespace game {

struct RenderDebugState {
    bool connected = false;
    std::uint32_t snapshotSequence = 0;
    std::uint64_t serverTick = 0;
    PlayerState player = {};
};

class OpenGLRenderer {
public:
    ~OpenGLRenderer();

    void setViewport(int width, int height);
    void render(const Camera& camera, const RenderDebugState& debugState);
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
