#pragma once

namespace game {

inline constexpr const char* debugLineVertexShader = R"glsl(
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

inline constexpr const char* debugLineFragmentShader = R"glsl(
#version 330 core
in vec4 vColor;
out vec4 outColor;
void main() {
    outColor = vColor;
}
)glsl";

} // namespace game
