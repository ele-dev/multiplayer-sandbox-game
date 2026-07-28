#include "client/graphics/SolidMeshRenderer.hpp"

#include "client/graphics/OpenGLFunctions.hpp"
#include "client/graphics/SolidColorShaders.hpp"

#include <SDL3/SDL_opengl.h>

#include <array>

namespace game {

namespace {

constexpr std::array<float, 108> cubeVertices = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,
};

} // namespace

SolidMeshRenderer::~SolidMeshRenderer() {
    shutdown();
}

bool SolidMeshRenderer::initialize() {
    shutdown();

    if (!shader_.initialize(solidColorVertexShader, solidColorFragmentShader)) {
        return false;
    }

    gl::genVertexArrays(1, &vertexArray_);
    gl::genBuffers(1, &vertexBuffer_);
    gl::bindVertexArray(vertexArray_);
    gl::bindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    gl::bufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(cubeVertices.size() * sizeof(float)),
        cubeVertices.data(),
        GL_STATIC_DRAW
    );
    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<void*>(0));
    return true;
}

void SolidMeshRenderer::shutdown() {
    if (vertexBuffer_ != 0) {
        gl::deleteBuffers(1, &vertexBuffer_);
        vertexBuffer_ = 0;
    }
    if (vertexArray_ != 0) {
        gl::deleteVertexArrays(1, &vertexArray_);
        vertexArray_ = 0;
    }
    shader_.shutdown();
}

void SolidMeshRenderer::drawCube(const glm::mat4& viewProjection, const glm::mat4& model, const glm::vec3& color) {
    shader_.use();
    shader_.setMat4("uMvp", viewProjection * model);
    shader_.setVec3("uColor", color);
    gl::bindVertexArray(vertexArray_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(cubeVertices.size() / 3));
}

bool SolidMeshRenderer::isInitialized() const {
    return shader_.isInitialized() && vertexArray_ != 0 && vertexBuffer_ != 0;
}

} // namespace game
