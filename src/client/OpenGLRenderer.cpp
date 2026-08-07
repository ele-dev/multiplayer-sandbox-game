#include "client/OpenGLRenderer.hpp"

#include "client/graphics/OpenGLFunctions.hpp"

#include <SDL3/SDL_opengl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace game {
namespace {

void addLine(std::vector<DebugLineVertex>& vertices, glm::vec3 start, glm::vec3 end, float r, float g, float b, float a) {
    vertices.push_back({start.x, start.y, start.z, r, g, b, a});
    vertices.push_back({end.x, end.y, end.z, r, g, b, a});
}

} // namespace

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

void OpenGLRenderer::clear() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::setViewport(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width_, height_);
}

void OpenGLRenderer::setObjectColor(glm::vec3 color) {
    objectColor_ = color;
}

void OpenGLRenderer::render(const Camera& camera) {
    if (!initialized_ && !initialize()) {
        glClearColor(0.15f, 0.02f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 worldMvp = camera.viewProjectionMatrix(width_, height_);
    const glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -5.0f));
    solidMeshRenderer_.drawCube(worldMvp, cubeModel, objectColor_);

    std::vector<DebugLineVertex> worldLines;
    worldLines.reserve(84);
    for (int i = -20; i <= 20; ++i) {
        const float color = (i == 0) ? 0.52f : 0.22f;
        addLine(
            worldLines,
            glm::vec3(-20.0f, 0.0f, static_cast<float>(i)),
            glm::vec3(20.0f, 0.0f, static_cast<float>(i)),
            color,
            color,
            color,
            1.0f
        );
        addLine(
            worldLines,
            glm::vec3(static_cast<float>(i), 0.0f, -20.0f),
            glm::vec3(static_cast<float>(i), 0.0f, 20.0f),
            color,
            color,
            color,
            1.0f
        );
    }
    addLine(worldLines, glm::vec3(0.0f, 0.02f, 0.0f), glm::vec3(2.0f, 0.02f, 0.0f), 0.95f, 0.18f, 0.18f, 1.0f);
    addLine(worldLines, glm::vec3(0.0f, 0.02f, 0.0f), glm::vec3(0.0f, 0.02f, 2.0f), 0.18f, 0.42f, 0.95f, 1.0f);
    debugLineRenderer_.draw(worldMvp, worldLines);
}

bool OpenGLRenderer::initialize() {
    if (!gl::loadFunctions()) {
        return false;
    }
    if (!solidMeshRenderer_.initialize()) {
        return false;
    }
    if (!debugLineRenderer_.initialize()) {
        solidMeshRenderer_.shutdown();
        return false;
    }
    initialized_ = true;
    return true;
}

void OpenGLRenderer::shutdown() {
    debugLineRenderer_.shutdown();
    solidMeshRenderer_.shutdown();
    initialized_ = false;
}

} // namespace game
