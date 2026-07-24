#include "client/OpenGLRenderer.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_video.h>

#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
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

struct Mat4 {
    std::array<float, 16> values{};
};

float dot(Vec3 a, Vec3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vec3 cross(Vec3 a, Vec3 b) {
    return {(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};
}

Vec3 normalize(Vec3 value) {
    const float len = std::sqrt(dot(value, value));
    if (len <= 0.0001f) {
        return {};
    }
    return {value.x / len, value.y / len, value.z / len};
}

Mat4 multiply(const Mat4& left, const Mat4& right) {
    Mat4 result;
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            float value = 0.0f;
            for (int i = 0; i < 4; ++i) {
                value += left.values[i * 4 + row] * right.values[column * 4 + i];
            }
            result.values[column * 4 + row] = value;
        }
    }
    return result;
}

Mat4 perspective(float fovRadians, float aspect, float nearPlane, float farPlane) {
    const float scale = 1.0f / std::tan(fovRadians * 0.5f);
    Mat4 result;
    result.values[0] = scale / aspect;
    result.values[5] = scale;
    result.values[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    result.values[11] = -1.0f;
    result.values[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    return result;
}

Mat4 orthographic(float left, float right, float bottom, float top) {
    Mat4 result;
    result.values[0] = 2.0f / (right - left);
    result.values[5] = 2.0f / (top - bottom);
    result.values[10] = -1.0f;
    result.values[12] = -(right + left) / (right - left);
    result.values[13] = -(top + bottom) / (top - bottom);
    result.values[15] = 1.0f;
    return result;
}

Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 worldUp) {
    const Vec3 forward = normalize({center.x - eye.x, center.y - eye.y, center.z - eye.z});
    const Vec3 side = normalize(cross(forward, worldUp));
    const Vec3 up = cross(side, forward);

    Mat4 result;
    result.values[0] = side.x;
    result.values[4] = side.y;
    result.values[8] = side.z;
    result.values[1] = up.x;
    result.values[5] = up.y;
    result.values[9] = up.z;
    result.values[2] = -forward.x;
    result.values[6] = -forward.y;
    result.values[10] = -forward.z;
    result.values[12] = -dot(side, eye);
    result.values[13] = -dot(up, eye);
    result.values[14] = dot(forward, eye);
    result.values[15] = 1.0f;
    return result;
}

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

void addLine(std::vector<Vertex>& vertices, Vec3 start, Vec3 end, float r, float g, float b, float a) {
    vertices.push_back({start.x, start.y, start.z, r, g, b, a});
    vertices.push_back({end.x, end.y, end.z, r, g, b, a});
}

void addQuad(
    std::vector<Vertex>& vertices,
    float x,
    float y,
    float width,
    float height,
    float r,
    float g,
    float b,
    float a
) {
    const float x2 = x + width;
    const float y2 = y + height;
    vertices.push_back({x, y, 0.0f, r, g, b, a});
    vertices.push_back({x2, y, 0.0f, r, g, b, a});
    vertices.push_back({x2, y2, 0.0f, r, g, b, a});
    vertices.push_back({x, y, 0.0f, r, g, b, a});
    vertices.push_back({x2, y2, 0.0f, r, g, b, a});
    vertices.push_back({x, y2, 0.0f, r, g, b, a});
}

std::array<std::string_view, 5> glyph(char character) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(character)))) {
    case 'A':
        return {"111", "101", "111", "101", "101"};
    case 'C':
        return {"111", "100", "100", "100", "111"};
    case 'D':
        return {"110", "101", "101", "101", "110"};
    case 'E':
        return {"111", "100", "110", "100", "111"};
    case 'H':
        return {"101", "101", "111", "101", "101"};
    case 'I':
        return {"111", "010", "010", "010", "111"};
    case 'K':
        return {"101", "101", "110", "101", "101"};
    case 'L':
        return {"100", "100", "100", "100", "111"};
    case 'N':
        return {"101", "111", "111", "111", "101"};
    case 'O':
        return {"111", "101", "101", "101", "111"};
    case 'P':
        return {"111", "101", "111", "100", "100"};
    case 'Q':
        return {"111", "101", "101", "111", "001"};
    case 'R':
        return {"110", "101", "110", "101", "101"};
    case 'S':
        return {"111", "100", "111", "001", "111"};
    case 'T':
        return {"111", "010", "010", "010", "010"};
    case 'V':
        return {"101", "101", "101", "101", "010"};
    case 'W':
        return {"101", "101", "111", "111", "101"};
    case 'Y':
        return {"101", "101", "010", "010", "010"};
    case '0':
        return {"111", "101", "101", "101", "111"};
    case '1':
        return {"010", "110", "010", "010", "111"};
    case '2':
        return {"111", "001", "111", "100", "111"};
    case '3':
        return {"111", "001", "111", "001", "111"};
    case '4':
        return {"101", "101", "111", "001", "001"};
    case '5':
        return {"111", "100", "111", "001", "111"};
    case '6':
        return {"111", "100", "111", "101", "111"};
    case '7':
        return {"111", "001", "001", "010", "010"};
    case '8':
        return {"111", "101", "111", "101", "111"};
    case '9':
        return {"111", "101", "111", "001", "111"};
    case ':':
        return {"000", "010", "000", "010", "000"};
    case '.':
        return {"000", "000", "000", "000", "010"};
    case '-':
        return {"000", "000", "111", "000", "000"};
    case '/':
        return {"001", "001", "010", "100", "100"};
    default:
        return {"000", "000", "000", "000", "000"};
    }
}

void addText(std::vector<Vertex>& vertices, float x, float y, std::string_view text, float scale) {
    float cursor = x;
    for (const char character : text) {
        if (character == ' ') {
            cursor += 4.0f * scale;
            continue;
        }

        const auto pattern = glyph(character);
        for (std::size_t row = 0; row < pattern.size(); ++row) {
            for (std::size_t column = 0; column < pattern[row].size(); ++column) {
                if (pattern[row][column] == '1') {
                    addQuad(
                        vertices,
                        cursor + static_cast<float>(column) * scale,
                        y + static_cast<float>(row) * scale,
                        scale,
                        scale,
                        0.78f,
                        0.92f,
                        1.0f,
                        0.92f
                    );
                }
            }
        }
        cursor += 4.0f * scale;
    }
}

void drawVertices(GLuint program, const Mat4& mvp, GLenum primitive, const std::vector<Vertex>& vertices) {
    if (vertices.empty()) {
        return;
    }

    glUseProgramPtr(program);
    const GLint mvpLocation = glGetUniformLocationPtr(program, "uMvp");
    glUniformMatrix4fvPtr(mvpLocation, 1, GL_FALSE, mvp.values.data());
    glBufferDataPtr(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );
    glDrawArrays(primitive, 0, static_cast<GLsizei>(vertices.size()));
}

std::string formatLine(const char* label, double value1, double value2 = 0.0, double value3 = 0.0, int count = 1) {
    std::array<char, 128> buffer{};
    if (count == 3) {
        std::snprintf(buffer.data(), buffer.size(), "%s: %.1f %.1f %.1f", label, value1, value2, value3);
    } else if (count == 2) {
        std::snprintf(buffer.data(), buffer.size(), "%s: %.2f/%.2f", label, value1, value2);
    } else {
        std::snprintf(buffer.data(), buffer.size(), "%s: %.0f", label, value1);
    }
    return buffer.data();
}

} // namespace

OpenGLRenderer::~OpenGLRenderer() {
    shutdown();
}

void OpenGLRenderer::setViewport(int width, int height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width_, height_);
}

void OpenGLRenderer::render(const Camera& camera, const RenderDebugState& debugState) {
    if (program_ == 0 && !initialize()) {
        glClearColor(0.15f, 0.02f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.04f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto& player = camera.player();
    const float cosPitch = std::cos(player.pitchRadians);
    const Vec3 eye = player.position;
    const Vec3 forward = {
        std::sin(player.yawRadians) * cosPitch,
        std::sin(player.pitchRadians),
        -std::cos(player.yawRadians) * cosPitch,
    };

    const float aspect = static_cast<float>(width_) / static_cast<float>(height_ > 0 ? height_ : 1);
    const Mat4 projection = perspective(70.0f * 3.14159265f / 180.0f, aspect, 0.05f, 200.0f);
    const Mat4 view = lookAt(eye, eye + forward, {0.0f, 1.0f, 0.0f});
    const Mat4 worldMvp = multiply(projection, view);

    std::vector<Vertex> worldLines;
    worldLines.reserve(164);
    for (int i = -20; i <= 20; ++i) {
        const float color = (i == 0) ? 0.52f : 0.22f;
        addLine(worldLines, {-20.0f, 0.0f, static_cast<float>(i)}, {20.0f, 0.0f, static_cast<float>(i)}, color, color, color, 1.0f);
        addLine(worldLines, {static_cast<float>(i), 0.0f, -20.0f}, {static_cast<float>(i), 0.0f, 20.0f}, color, color, color, 1.0f);
    }
    addLine(worldLines, {0.0f, 0.02f, 0.0f}, {2.0f, 0.02f, 0.0f}, 0.95f, 0.18f, 0.18f, 1.0f);
    addLine(worldLines, {0.0f, 0.02f, 0.0f}, {0.0f, 0.02f, 2.0f}, 0.18f, 0.42f, 0.95f, 1.0f);
    drawVertices(program_, worldMvp, GL_LINES, worldLines);

    glDisable(GL_DEPTH_TEST);
    const Mat4 screenMvp = orthographic(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f);

    std::vector<Vertex> screenLines;
    const float centerX = static_cast<float>(width_) * 0.5f;
    const float centerY = static_cast<float>(height_) * 0.5f;
    addLine(screenLines, {centerX - 14.0f, centerY, 0.0f}, {centerX - 4.0f, centerY, 0.0f}, 0.85f, 0.95f, 1.0f, 0.9f);
    addLine(screenLines, {centerX + 4.0f, centerY, 0.0f}, {centerX + 14.0f, centerY, 0.0f}, 0.85f, 0.95f, 1.0f, 0.9f);
    addLine(screenLines, {centerX, centerY - 14.0f, 0.0f}, {centerX, centerY - 4.0f, 0.0f}, 0.85f, 0.95f, 1.0f, 0.9f);
    addLine(screenLines, {centerX, centerY + 4.0f, 0.0f}, {centerX, centerY + 14.0f, 0.0f}, 0.85f, 0.95f, 1.0f, 0.9f);
    drawVertices(program_, screenMvp, GL_LINES, screenLines);

    std::vector<Vertex> overlay;
    constexpr float panelWidth = 350.0f;
    constexpr float panelHeight = 132.0f;
    const float panelX = static_cast<float>(width_) - panelWidth - 16.0f;
    const float panelY = 16.0f;
    addQuad(overlay, panelX, panelY, panelWidth, panelHeight, 0.02f, 0.03f, 0.04f, 0.62f);
    addQuad(overlay, panelX, panelY, panelWidth, 2.0f, 0.22f, 0.62f, 0.95f, 0.8f);

    constexpr float textScale = 4.0f;
    float textY = panelY + 14.0f;
    addText(overlay, panelX + 14.0f, textY, debugState.connected ? "CONNECTED: YES" : "CONNECTED: NO", textScale);
    textY += 22.0f;
    addText(overlay, panelX + 14.0f, textY, formatLine("SERVER TICK", static_cast<double>(debugState.serverTick)), textScale);
    textY += 22.0f;
    addText(
        overlay,
        panelX + 14.0f,
        textY,
        formatLine(
            "POS",
            static_cast<double>(debugState.player.position.x),
            static_cast<double>(debugState.player.position.y),
            static_cast<double>(debugState.player.position.z),
            3
        ),
        textScale
    );
    textY += 22.0f;
    addText(
        overlay,
        panelX + 14.0f,
        textY,
        formatLine(
            "YAW/PITCH",
            static_cast<double>(debugState.player.yawRadians),
            static_cast<double>(debugState.player.pitchRadians),
            0.0,
            2
        ),
        textScale
    );
    textY += 22.0f;
    addText(overlay, panelX + 14.0f, textY, formatLine("SNAPSHOT", static_cast<double>(debugState.snapshotSequence)), textScale);
    drawVertices(program_, screenMvp, GL_TRIANGLES, overlay);
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
