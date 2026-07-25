#pragma once

#include "shared/PlayerState.hpp"
#include "shared/Protocol.hpp"

#include <cstdint>

namespace game {

class Simulation {
public:
    void applyInput(const ClientInputCommand& input);
    void tick(float fixedDeltaSeconds);

    void setMoveSpeed(float speed);
    void setMouseSensitivity(float sensitivity);
    void setMaxPitchRadians(float radians);

    [[nodiscard]] const PlayerState& player() const;
    [[nodiscard]] std::uint64_t tickCount() const;

private:
    PlayerState player_ = {.playerId = 1};
    ClientInputCommand latestInput_ = {};
    std::uint64_t tickCount_ = 0;
    float moveSpeed_ = 4.5f;
    float mouseSensitivity_ = 0.0025f;
    float maxPitchRadians_ = 1.5f;
};

} // namespace game
