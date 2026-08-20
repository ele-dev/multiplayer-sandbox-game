#include "client/Camera.hpp"

#include <gtest/gtest.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <cstddef>

namespace game {
namespace {

constexpr float epsilon = 1.0e-5f;

void expectVectorNear(const glm::vec3& actual, const glm::vec3& expected) {
    EXPECT_NEAR(actual.x, expected.x, epsilon);
    EXPECT_NEAR(actual.y, expected.y, epsilon);
    EXPECT_NEAR(actual.z, expected.z, epsilon);
}

void expectMatrixNear(const glm::mat4& actual, const glm::mat4& expected) {
    for (std::size_t column = 0; column < 4; ++column) {
        for (std::size_t row = 0; row < 4; ++row) {
            EXPECT_NEAR(actual[column][row], expected[column][row], epsilon)
                << "matrix element [" << column << "][" << row << "]";
        }
    }
}

TEST(CameraTest, ForwardDirectionMatchesKnownYawAndPitchValues) {
    Camera camera;
    PlayerState player;

    camera.setFromPlayer(player);
    expectVectorNear(camera.forwardDirection(), {0.0f, 0.0f, -1.0f});

    player.yawRadians = glm::half_pi<float>();
    camera.setFromPlayer(player);
    expectVectorNear(camera.forwardDirection(), {1.0f, 0.0f, 0.0f});

    player.pitchRadians = glm::pi<float>() / 6.0f;
    camera.setFromPlayer(player);
    expectVectorNear(camera.forwardDirection(), {0.8660254f, 0.5f, 0.0f});
}

TEST(CameraTest, ViewMatrixMatchesLookAt) {
    Camera camera;
    PlayerState player;
    player.position = {3.0f, 2.0f, -5.0f};
    player.yawRadians = 0.7f;
    player.pitchRadians = -0.2f;
    camera.setFromPlayer(player);

    const float cosPitch = std::cos(player.pitchRadians);
    const glm::vec3 expectedDirection = {
        std::sin(player.yawRadians) * cosPitch,
        std::sin(player.pitchRadians),
        -std::cos(player.yawRadians) * cosPitch,
    };
    const glm::mat4 expected = glm::lookAt(
        player.position,
        player.position + expectedDirection,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    expectMatrixNear(camera.viewMatrix(), expected);
}

TEST(CameraTest, ProjectionMatrixUsesConfiguredParameters) {
    Camera camera;
    constexpr float aspect = 16.0f / 9.0f;
    const glm::mat4 expected = glm::perspective(glm::radians(70.0f), aspect, 0.05f, 200.0f);

    expectMatrixNear(camera.projectionMatrix(aspect), expected);
}

TEST(CameraTest, ViewProjectionCombinesProjectionAndView) {
    Camera camera;
    constexpr int width = 1920;
    constexpr int height = 1080;
    const float aspect = static_cast<float>(width) / static_cast<float>(height);

    expectMatrixNear(
        camera.viewProjectionMatrix(width, height),
        camera.projectionMatrix(aspect) * camera.viewMatrix()
    );
}

} // namespace
} // namespace game
