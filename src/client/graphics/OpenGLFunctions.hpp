#pragma once

#include <SDL3/SDL_opengl.h>

namespace game::gl {

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
using GlUniform3f = void (*)(GLint, GLfloat, GLfloat, GLfloat);
using GlUniformMatrix4fv = void (*)(GLint, GLsizei, GLboolean, const GLfloat*);

extern GlCreateShader createShader;
extern GlShaderSource shaderSource;
extern GlCompileShader compileShader;
extern GlGetShaderiv getShaderiv;
extern GlGetShaderInfoLog getShaderInfoLog;
extern GlCreateProgram createProgram;
extern GlAttachShader attachShader;
extern GlLinkProgram linkProgram;
extern GlGetProgramiv getProgramiv;
extern GlGetProgramInfoLog getProgramInfoLog;
extern GlDeleteShader deleteShader;
extern GlDeleteProgram deleteProgram;
extern GlUseProgram useProgram;
extern GlGenVertexArrays genVertexArrays;
extern GlBindVertexArray bindVertexArray;
extern GlDeleteVertexArrays deleteVertexArrays;
extern GlGenBuffers genBuffers;
extern GlBindBuffer bindBuffer;
extern GlBufferData bufferData;
extern GlDeleteBuffers deleteBuffers;
extern GlEnableVertexAttribArray enableVertexAttribArray;
extern GlVertexAttribPointer vertexAttribPointer;
extern GlGetUniformLocation getUniformLocation;
extern GlUniform3f uniform3f;
extern GlUniformMatrix4fv uniformMatrix4fv;

bool loadFunctions();

} // namespace game::gl
