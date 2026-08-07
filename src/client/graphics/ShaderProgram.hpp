#pragma once

#include <SDL3/SDL_opengl.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace game {

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    bool initialize(const char* vertexSource, const char* fragmentSource);
    void shutdown();

    void use() const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setMat4(const char* name, const glm::mat4& value) const;

    [[nodiscard]] GLuint id() const { return program_; }
    [[nodiscard]] bool isInitialized() const { return program_ != 0; }

private:
    GLuint program_ = 0;
};

} // namespace game
