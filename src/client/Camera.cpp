#include "client/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace game {

namespace {

constexpr float fieldOfViewDegrees = 70.0f;
constexpr float nearPlane = 0.05f;
constexpr float farPlane = 200.0f;

} // namespace

void Camera::setFromPlayer(const PlayerState& player) {
    player_ = player;
}

const PlayerState& Camera::player() const {
    return player_;
}

glm::vec3 Camera::position() const {
    return player_.position;
}

glm::vec3 Camera::forwardDirection() const {
    const float cosPitch = std::cos(player_.pitchRadians);
    return {
        std::sin(player_.yawRadians) * cosPitch,
        std::sin(player_.pitchRadians),
        -std::cos(player_.yawRadians) * cosPitch,
    };
}

glm::vec3 Camera::target() const {
    return position() + forwardDirection();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(position(), target(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(fieldOfViewDegrees), aspect, nearPlane, farPlane);
}

glm::mat4 Camera::viewProjectionMatrix(int width, int height) const {
    const float aspect = static_cast<float>(width) / static_cast<float>(height > 0 ? height : 1);
    return projectionMatrix(aspect) * viewMatrix();
}

} // namespace game
