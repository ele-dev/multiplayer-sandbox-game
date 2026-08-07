#include "client/graphics/ShaderProgram.hpp"

#include "client/graphics/OpenGLFunctions.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <iostream>

namespace game {

namespace {

GLuint compileShader(GLenum type, const char* source) {
    const GLuint shader = gl::createShader(type);
    gl::shaderSource(shader, 1, &source, nullptr);
    gl::compileShader(shader);

    GLint success = 0;
    gl::getShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    std::array<char, 1024> log{};
    gl::getShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::cerr << "OpenGL shader compile failed: " << log.data() << '\n';
    gl::deleteShader(shader);
    return 0;
}

} // namespace

ShaderProgram::~ShaderProgram() {
    shutdown();
}

bool ShaderProgram::initialize(const char* vertexSource, const char* fragmentSource) {
    shutdown();

    const GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vertexShader == 0 || fragmentShader == 0) {
        if (vertexShader != 0) {
            gl::deleteShader(vertexShader);
        }
        if (fragmentShader != 0) {
            gl::deleteShader(fragmentShader);
        }
        return false;
    }

    const GLuint program = gl::createProgram();
    gl::attachShader(program, vertexShader);
    gl::attachShader(program, fragmentShader);
    gl::linkProgram(program);
    gl::deleteShader(vertexShader);
    gl::deleteShader(fragmentShader);

    GLint success = 0;
    gl::getProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE) {
        program_ = program;
        return true;
    }

    std::array<char, 1024> log{};
    gl::getProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::cerr << "OpenGL program link failed: " << log.data() << '\n';
    gl::deleteProgram(program);
    return false;
}

void ShaderProgram::shutdown() {
    if (program_ != 0) {
        gl::deleteProgram(program_);
        program_ = 0;
    }
}

void ShaderProgram::use() const {
    gl::useProgram(program_);
}

void ShaderProgram::setVec3(const char* name, const glm::vec3& value) const {
    const GLint location = gl::getUniformLocation(program_, name);
    gl::uniform3f(location, value.x, value.y, value.z);
}

void ShaderProgram::setMat4(const char* name, const glm::mat4& value) const {
    const GLint location = gl::getUniformLocation(program_, name);
    gl::uniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace game
