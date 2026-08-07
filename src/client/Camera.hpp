#pragma once

#include "shared/PlayerState.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace game {

class Camera {
public:
    void setFromPlayer(const PlayerState& player);

    [[nodiscard]] const PlayerState& player() const;
    [[nodiscard]] glm::vec3 position() const;
    [[nodiscard]] glm::vec3 forwardDirection() const;
    [[nodiscard]] glm::vec3 target() const;
    [[nodiscard]] glm::mat4 viewMatrix() const;
    [[nodiscard]] glm::mat4 projectionMatrix(float aspect) const;
    [[nodiscard]] glm::mat4 viewProjectionMatrix(int width, int height) const;

private:
    PlayerState player_;
};

} // namespace game
