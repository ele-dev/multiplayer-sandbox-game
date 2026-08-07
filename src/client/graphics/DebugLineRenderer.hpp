#pragma once

#include "client/graphics/ShaderProgram.hpp"

#include <glm/mat4x4.hpp>

#include <vector>

namespace game {

struct DebugLineVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

class DebugLineRenderer {
public:
    DebugLineRenderer() = default;
    ~DebugLineRenderer();

    DebugLineRenderer(const DebugLineRenderer&) = delete;
    DebugLineRenderer& operator=(const DebugLineRenderer&) = delete;

    bool initialize();
    void shutdown();
    void draw(const glm::mat4& viewProjection, const std::vector<DebugLineVertex>& vertices);

    [[nodiscard]] bool isInitialized() const;

private:
    ShaderProgram shader_;
    unsigned int vertexArray_ = 0;
    unsigned int vertexBuffer_ = 0;
};

} // namespace game
