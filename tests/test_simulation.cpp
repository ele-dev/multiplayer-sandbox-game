#include "shared/Simulation.hpp"

#include <gtest/gtest.h>

#include <glm/geometric.hpp>

namespace game {
namespace {

constexpr float epsilon = 1.0e-5f;

TEST(SimulationTest, RepeatsLatestInputDeterministicallyAcrossTicks) {
    Simulation simulation;
    ClientInputCommand input;
    input.movement = {0.0f, 1.0f};
    simulation.applyInput(input);

    for (int i = 0; i < 10; ++i) {
        simulation.tick(1.0f / 60.0f);
    }

    EXPECT_EQ(simulation.tickCount(), 10U);
    EXPECT_NEAR(simulation.player().position.x, 0.0f, epsilon);
    EXPECT_NEAR(simulation.player().position.y, 1.8f, epsilon);
    EXPECT_NEAR(simulation.player().position.z, -0.75f, epsilon);
    EXPECT_NEAR(simulation.player().yawRadians, 0.0f, epsilon);
    EXPECT_NEAR(simulation.player().pitchRadians, 0.0f, epsilon);
}

TEST(SimulationTest, ZeroLengthMovementDoesNotChangePosition) {
    Simulation simulation;
    ClientInputCommand input;
    input.lookDelta = {2.0f, 1.0f};
    simulation.applyInput(input);

    simulation.tick(1.0f / 60.0f);

    EXPECT_EQ(simulation.player().position, glm::vec3(0.0f, 1.8f, 0.0f));
    EXPECT_NEAR(simulation.player().yawRadians, 0.005f, epsilon);
    EXPECT_NEAR(simulation.player().pitchRadians, 0.0025f, epsilon);
}

TEST(SimulationTest, NormalizesDiagonalMovement) {
    Simulation simulation;
    ClientInputCommand input;
    input.movement = {1.0f, 1.0f};
    simulation.applyInput(input);

    simulation.tick(1.0f);

    const glm::vec2 horizontalDisplacement = {
        simulation.player().position.x,
        simulation.player().position.z,
    };
    EXPECT_NEAR(glm::length(horizontalDisplacement), 4.5f, epsilon);
}

TEST(SimulationTest, ClampsPitchAtConfiguredLimits) {
    Simulation simulation;
    simulation.setMaxPitchRadians(0.5f);

    ClientInputCommand input;
    input.lookDelta.y = 1000.0f;
    simulation.applyInput(input);
    simulation.tick(1.0f / 60.0f);
    EXPECT_FLOAT_EQ(simulation.player().pitchRadians, 0.5f);

    input.lookDelta.y = -1000.0f;
    simulation.applyInput(input);
    simulation.tick(1.0f / 60.0f);
    EXPECT_FLOAT_EQ(simulation.player().pitchRadians, -0.5f);
}

TEST(SimulationTest, RuntimeSettingsAffectTheNextTick) {
    Simulation simulation;
    ClientInputCommand input;
    input.movement.y = 1.0f;
    input.lookDelta.x = 1.0f;
    simulation.applyInput(input);

    simulation.setMoveSpeed(2.0f);
    simulation.setMouseSensitivity(0.1f);
    simulation.tick(0.5f);

    EXPECT_NEAR(simulation.player().yawRadians, 0.1f, epsilon);
    EXPECT_NEAR(glm::length(glm::vec2(
        simulation.player().position.x,
        simulation.player().position.z
    )), 1.0f, epsilon);

    simulation.setMoveSpeed(4.0f);
    simulation.setMouseSensitivity(0.2f);
    const glm::vec3 previousPosition = simulation.player().position;
    simulation.tick(0.5f);

    EXPECT_NEAR(simulation.player().yawRadians, 0.3f, epsilon);
    EXPECT_NEAR(glm::length(glm::vec2(
        simulation.player().position.x - previousPosition.x,
        simulation.player().position.z - previousPosition.z
    )), 2.0f, epsilon);
}

} // namespace
} // namespace game
