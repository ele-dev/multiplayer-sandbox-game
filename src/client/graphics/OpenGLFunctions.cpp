#include "client/graphics/OpenGLFunctions.hpp"

#include <SDL3/SDL_video.h>

#include <iostream>

namespace game::gl {

GlCreateShader createShader = nullptr;
GlShaderSource shaderSource = nullptr;
GlCompileShader compileShader = nullptr;
GlGetShaderiv getShaderiv = nullptr;
GlGetShaderInfoLog getShaderInfoLog = nullptr;
GlCreateProgram createProgram = nullptr;
GlAttachShader attachShader = nullptr;
GlLinkProgram linkProgram = nullptr;
GlGetProgramiv getProgramiv = nullptr;
GlGetProgramInfoLog getProgramInfoLog = nullptr;
GlDeleteShader deleteShader = nullptr;
GlDeleteProgram deleteProgram = nullptr;
GlUseProgram useProgram = nullptr;
GlGenVertexArrays genVertexArrays = nullptr;
GlBindVertexArray bindVertexArray = nullptr;
GlDeleteVertexArrays deleteVertexArrays = nullptr;
GlGenBuffers genBuffers = nullptr;
GlBindBuffer bindBuffer = nullptr;
GlBufferData bufferData = nullptr;
GlDeleteBuffers deleteBuffers = nullptr;
GlEnableVertexAttribArray enableVertexAttribArray = nullptr;
GlVertexAttribPointer vertexAttribPointer = nullptr;
GlGetUniformLocation getUniformLocation = nullptr;
GlUniform3f uniform3f = nullptr;
GlUniformMatrix4fv uniformMatrix4fv = nullptr;

namespace {

template <typename T>
bool loadGl(T& target, const char* name) {
    target = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (target == nullptr) {
        std::cerr << "Missing OpenGL function: " << name << '\n';
        return false;
    }
    return true;
}

} // namespace

bool loadFunctions() {
    static bool loaded = false;
    if (loaded) {
        return true;
    }

    loaded = loadGl(createShader, "glCreateShader") && loadGl(shaderSource, "glShaderSource") &&
             loadGl(compileShader, "glCompileShader") && loadGl(getShaderiv, "glGetShaderiv") &&
             loadGl(getShaderInfoLog, "glGetShaderInfoLog") && loadGl(createProgram, "glCreateProgram") &&
             loadGl(attachShader, "glAttachShader") && loadGl(linkProgram, "glLinkProgram") &&
             loadGl(getProgramiv, "glGetProgramiv") && loadGl(getProgramInfoLog, "glGetProgramInfoLog") &&
             loadGl(deleteShader, "glDeleteShader") && loadGl(deleteProgram, "glDeleteProgram") &&
             loadGl(useProgram, "glUseProgram") && loadGl(genVertexArrays, "glGenVertexArrays") &&
             loadGl(bindVertexArray, "glBindVertexArray") && loadGl(deleteVertexArrays, "glDeleteVertexArrays") &&
             loadGl(genBuffers, "glGenBuffers") && loadGl(bindBuffer, "glBindBuffer") &&
             loadGl(bufferData, "glBufferData") && loadGl(deleteBuffers, "glDeleteBuffers") &&
             loadGl(enableVertexAttribArray, "glEnableVertexAttribArray") &&
             loadGl(vertexAttribPointer, "glVertexAttribPointer") &&
             loadGl(getUniformLocation, "glGetUniformLocation") && loadGl(uniform3f, "glUniform3f") &&
             loadGl(uniformMatrix4fv, "glUniformMatrix4fv");
    return loaded;
}

} // namespace game::gl
