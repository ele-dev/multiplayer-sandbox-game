#include "shared/Simulation.hpp"

#include "shared/Math.hpp"

#include <cmath>

namespace game {

void Simulation::applyInput(const ClientInputCommand& input) {
    latestInput_ = input;
}

void Simulation::tick(float fixedDeltaSeconds) {
    player_.yawRadians += latestInput_.lookDelta.x * mouseSensitivity_;
    player_.pitchRadians = clamp(
        player_.pitchRadians + latestInput_.lookDelta.y * mouseSensitivity_,
        -maxPitchRadians_,
        maxPitchRadians_
    );

    const Vec2 movement = normalize(latestInput_.movement);
    const float sinYaw = std::sin(player_.yawRadians);
    const float cosYaw = std::cos(player_.yawRadians);

    const Vec3 forward = {sinYaw, 0.0f, -cosYaw};
    const Vec3 right = {cosYaw, 0.0f, sinYaw};
    const Vec3 velocity = (forward * movement.y) + (right * movement.x);

    player_.position = player_.position + (velocity * (moveSpeed_ * fixedDeltaSeconds));
    ++tickCount_;
}

void Simulation::setMoveSpeed(float speed) {
    moveSpeed_ = speed;
}

void Simulation::setMouseSensitivity(float sensitivity) {
    mouseSensitivity_ = sensitivity;
}

void Simulation::setMaxPitchRadians(float radians) {
    maxPitchRadians_ = radians;
}

const PlayerState& Simulation::player() const {
    return player_;
}

std::uint64_t Simulation::tickCount() const {
    return tickCount_;
}

} // namespace game
