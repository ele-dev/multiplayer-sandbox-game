#include "client/graphics/DebugLineRenderer.hpp"

#include "client/graphics/DebugLineShaders.hpp"
#include "client/graphics/OpenGLFunctions.hpp"

#include <SDL3/SDL_opengl.h>

namespace game {

DebugLineRenderer::~DebugLineRenderer() {
    shutdown();
}

bool DebugLineRenderer::initialize() {
    shutdown();

    if (!shader_.initialize(debugLineVertexShader, debugLineFragmentShader)) {
        return false;
    }

    gl::genVertexArrays(1, &vertexArray_);
    gl::genBuffers(1, &vertexBuffer_);
    gl::bindVertexArray(vertexArray_);
    gl::bindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    gl::enableVertexAttribArray(0);
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugLineVertex), reinterpret_cast<void*>(0));
    gl::enableVertexAttribArray(1);
    gl::vertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugLineVertex), reinterpret_cast<void*>(sizeof(float) * 3));
    return true;
}

void DebugLineRenderer::shutdown() {
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

void DebugLineRenderer::draw(const glm::mat4& viewProjection, const std::vector<DebugLineVertex>& vertices) {
    if (vertices.empty()) {
        return;
    }

    shader_.use();
    shader_.setMat4("uMvp", viewProjection);
    gl::bindVertexArray(vertexArray_);
    gl::bindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
    gl::bufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(DebugLineVertex)),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
}

bool DebugLineRenderer::isInitialized() const {
    return shader_.isInitialized() && vertexArray_ != 0 && vertexBuffer_ != 0;
}

} // namespace game
