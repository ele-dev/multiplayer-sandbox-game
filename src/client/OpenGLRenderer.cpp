#include "client/OpenGLRenderer.hpp"

#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

namespace game {
namespace {

using GlCreateShader = GLuint (*)(GLenum);
using GlShaderSource = void (*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using GlCompileShader = void (*)(GLuint);
using GlGetShaderiv = void (*)(GLuint, GLenum, GLint*);
using GlGetShaderInfoLog = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using GlCreateProgram = GLuint (*)();
using GlAttachShader = void (*)(GLuint, GLuint);
using GlLinkProgram = void (*)(GLuint);
using GlGetProgramiv = void (*)(GLuint, GLenum, GLint*);
using GlGetProgramInfoLog = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using GlDeleteShader = void (*)(GLuint);
using GlDeleteProgram = void (*)(GLuint);
using GlUseProgram = void (*)(GLuint);
using GlGenVertexArrays = void (*)(GLsizei, GLuint*);
using GlBindVertexArray = void (*)(GLuint);
using GlDeleteVertexArrays = void (*)(GLsizei, const GLuint*);
using GlGenBuffers = void (*)(GLsizei, GLuint*);
using GlBindBuffer = void (*)(GLenum, GLuint);
using GlBufferData = void (*)(GLenum, GLsizeiptr, const void*, GLenum);
using GlDeleteBuffers = void (*)(GLsizei, const GLuint*);
using GlEnableVertexAttribArray = void (*)(GLuint);
using GlVertexAttribPointer = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
using GlGetUniformLocation = GLint (*)(GLuint, const GLchar*);
using GlUniformMatrix4fv = void (*)(GLint, GLsizei, GLboolean, const GLfloat*);

GlCreateShader glCreateShaderPtr = nullptr;
GlShaderSource glShaderSourcePtr = nullptr;
GlCompileShader glCompileShaderPtr = nullptr;
GlGetShaderiv glGetShaderivPtr = nullptr;
GlGetShaderInfoLog glGetShaderInfoLogPtr = nullptr;
GlCreateProgram glCreateProgramPtr = nullptr;
GlAttachShader glAttachShaderPtr = nullptr;
GlLinkProgram glLinkProgramPtr = nullptr;
GlGetProgramiv glGetProgramivPtr = nullptr;
GlGetProgramInfoLog glGetProgramInfoLogPtr = nullptr;
GlDeleteShader glDeleteShaderPtr = nullptr;
GlDeleteProgram glDeleteProgramPtr = nullptr;
GlUseProgram glUseProgramPtr = nullptr;
GlGenVertexArrays glGenVertexArraysPtr = nullptr;
GlBindVertexArray glBindVertexArrayPtr = nullptr;
GlDeleteVertexArrays glDeleteVertexArraysPtr = nullptr;
GlGenBuffers glGenBuffersPtr = nullptr;
GlBindBuffer glBindBufferPtr = nullptr;
GlBufferData glBufferDataPtr = nullptr;
GlDeleteBuffers glDeleteBuffersPtr = nullptr;
GlEnableVertexAttribArray glEnableVertexAttribArrayPtr = nullptr;
GlVertexAttribPointer glVertexAttribPointerPtr = nullptr;
GlGetUniformLocation glGetUniformLocationPtr = nullptr;
GlUniformMatrix4fv glUniformMatrix4fvPtr = nullptr;

template <typename T>
bool loadGl(T& target, const char* name) {
    target = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (target == nullptr) {
        std::cerr << "Missing OpenGL function: " << name << '\n';
        return false;
    }
    return true;
}

bool loadGlFunctions() {
    return loadGl(glCreateShaderPtr, "glCreateShader") && loadGl(glShaderSourcePtr, "glShaderSource") &&
           loadGl(glCompileShaderPtr, "glCompileShader") && loadGl(glGetShaderivPtr, "glGetShaderiv") &&
           loadGl(glGetShaderInfoLogPtr, "glGetShaderInfoLog") && loadGl(glCreateProgramPtr, "glCreateProgram") &&
           loadGl(glAttachShaderPtr, "glAttachShader") && loadGl(glLinkProgramPtr, "glLinkProgram") &&
           loadGl(glGetProgramivPtr, "glGetProgramiv") &&
           loadGl(glGetProgramInfoLogPtr, "glGetProgramInfoLog") && loadGl(glDeleteShaderPtr, "glDeleteShader") &&
           loadGl(glDeleteProgramPtr, "glDeleteProgram") && loadGl(glUseProgramPtr, "glUseProgram") &&
           loadGl(glGenVertexArraysPtr, "glGenVertexArrays") && loadGl(glBindVertexArrayPtr, "glBindVertexArray") &&
           loadGl(glDeleteVertexArraysPtr, "glDeleteVertexArrays") && loadGl(glGenBuffersPtr, "glGenBuffers") &&
           loadGl(glBindBufferPtr, "glBindBuffer") && loadGl(glBufferDataPtr, "glBufferData") &&
           loadGl(glDeleteBuffersPtr, "glDeleteBuffers") &&
           loadGl(glEnableVertexAttribArrayPtr, "glEnableVertexAttribArray") &&
           loadGl(glVertexAttribPointerPtr, "glVertexAttribPointer") &&
           loadGl(glGetUniformLocationPtr, "glGetUniformLocation") &&
           loadGl(glUniformMatrix4fvPtr, "glUniformMatrix4fv");
}

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

GLuint compileShader(GLenum type, const char* source) {
    const GLuint shader = glCreateShaderPtr(type);
    glShaderSourcePtr(shader, 1, &source, nullptr);
    glCompileShaderPtr(shader);

    GLint success = 0;
    glGetShaderivPtr(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    std::array<char, 1024> log{};
    glGetShaderInfoLogPtr(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::cerr << "OpenGL shader compile failed: " << log.data() << '\n';
    glDeleteShaderPtr(shader);
    return 0;
}

GLuint createProgram() {
    constexpr const char* vertexSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
uniform mat4 uMvp;
out vec4 vColor;
void main() {
    vColor = aColor;
    gl_Position = uMvp * vec4(aPosition, 1.0);
}
)glsl";

    constexpr const char* fragmentSource = R"glsl(
#version 330 core
in vec4 vColor;
out vec4 outColor;
void main() {
    outColor = vColor;
}
)glsl";

    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        return 0;
    }

    const GLuint program = glCreateProgramPtr();
    glAttachShaderPtr(program, vertexShader);
    glAttachShaderPtr(program, fragmentShader);
    glLinkProgramPtr(program);
    glDeleteShaderPtr(vertexShader);
    glDeleteShaderPtr(fragmentShader);

    GLint success = 0;
    glGetProgramivPtr(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE) {
        return program;
    }

    std::array<char, 1024> log{};
    glGetProgramInfoLogPtr(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::cerr << "OpenGL program link failed: " << log.data() << '\n';
    glDeleteProgramPtr(program);
    return 0;
}

void addLine(std::vector<Vertex>& vertices, glm::vec3 start, glm::vec3 end, float r, float g, float b, float a) {
    vertices.push_back({start.x, start.y, start.z, r, g, b, a});
    vertices.push_back({end.x, end.y, end.z, r, g, b, a});
}

void drawVertices(GLuint program, const glm::mat4& mvp, GLenum primitive, const std::vector<Vertex>& vertices) {
    if (vertices.empty()) {
        return;
    }

    glUseProgramPtr(program);
    const GLint mvpLocation = glGetUniformLocationPtr(program, "uMvp");
    glUniformMatrix4fvPtr(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glBufferDataPtr(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );
    glDrawArrays(primitive, 0, static_cast<GLsizei>(vertices.size()));
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

void OpenGLRenderer::render(const Camera& camera) {
    if (program_ == 0 && !initialize()) {
        glClearColor(0.15f, 0.02f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.04f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto& player = camera.player();
    const float cosPitch = std::cos(player.pitchRadians);
    const glm::vec3 eye = player.position;
    const glm::vec3 forward = {
        std::sin(player.yawRadians) * cosPitch,
        std::sin(player.pitchRadians),
        -std::cos(player.yawRadians) * cosPitch,
    };

    const float aspect = static_cast<float>(width_) / static_cast<float>(height_ > 0 ? height_ : 1);
    const glm::mat4 projection = glm::perspective(glm::radians(70.0f), aspect, 0.05f, 200.0f);
    const glm::mat4 view = glm::lookAt(eye, eye + forward, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 worldMvp = projection * view;

    std::vector<Vertex> worldLines;
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
    drawVertices(program_, worldMvp, GL_LINES, worldLines);
}

bool OpenGLRenderer::initialize() {
    if (!loadGlFunctions()) {
        return false;
    }

    program_ = createProgram();
    if (program_ == 0) {
        return false;
    }

    glGenVertexArraysPtr(1, &vertexArray_);
    glGenBuffersPtr(1, &vertexBuffer_);
    glBindVertexArrayPtr(vertexArray_);
    glBindBufferPtr(GL_ARRAY_BUFFER, vertexBuffer_);
    glEnableVertexAttribArrayPtr(0);
    glVertexAttribPointerPtr(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArrayPtr(1);
    glVertexAttribPointerPtr(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(float) * 3));
    return true;
}

void OpenGLRenderer::shutdown() {
    if (vertexBuffer_ != 0) {
        glDeleteBuffersPtr(1, &vertexBuffer_);
        vertexBuffer_ = 0;
    }
    if (vertexArray_ != 0) {
        glDeleteVertexArraysPtr(1, &vertexArray_);
        vertexArray_ = 0;
    }
    if (program_ != 0) {
        glDeleteProgramPtr(program_);
        program_ = 0;
    }
}

} // namespace game
