#pragma once

#include "shared/PlayerState.hpp"

namespace game {

class Camera {
public:
    void setFromPlayer(const PlayerState& player);

    [[nodiscard]] const PlayerState& player() const;

private:
    PlayerState player_;
};

} // namespace game
