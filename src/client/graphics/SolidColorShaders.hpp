#pragma once

namespace game {

inline constexpr const char* solidColorVertexShader = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPosition;
uniform mat4 uMvp;
void main() {
    gl_Position = uMvp * vec4(aPosition, 1.0);
}
)glsl";

inline constexpr const char* solidColorFragmentShader = R"glsl(
#version 330 core
uniform vec3 uColor;
out vec4 outColor;
void main() {
    outColor = vec4(uColor, 1.0);
}
)glsl";

} // namespace game
