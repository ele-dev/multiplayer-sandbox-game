#include "client/Camera.hpp"

namespace game {

void Camera::setFromPlayer(const PlayerState& player) {
    player_ = player;
}

const PlayerState& Camera::player() const {
    return player_;
}

} // namespace game
