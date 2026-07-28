#pragma once

#include "client/graphics/ShaderProgram.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace game {

class SolidMeshRenderer {
public:
    SolidMeshRenderer() = default;
    ~SolidMeshRenderer();

    SolidMeshRenderer(const SolidMeshRenderer&) = delete;
    SolidMeshRenderer& operator=(const SolidMeshRenderer&) = delete;

    bool initialize();
    void shutdown();
    void drawCube(const glm::mat4& viewProjection, const glm::mat4& model, const glm::vec3& color);

    [[nodiscard]] bool isInitialized() const;

private:
    ShaderProgram shader_;
    unsigned int vertexArray_ = 0;
    unsigned int vertexBuffer_ = 0;
};

} // namespace game
