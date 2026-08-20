#include "net/Serialization.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace game {
namespace {

template <typename Deserialize>
void expectEveryTruncationRejected(const std::vector<std::uint8_t>& packet, Deserialize deserialize) {
    for (std::size_t size = 0; size < packet.size(); ++size) {
        const std::vector<std::uint8_t> truncated(packet.begin(), packet.begin() + size);
        EXPECT_FALSE(deserialize(truncated).has_value()) << "accepted size " << size;
    }
}

TEST(SerializationTest, ClientHelloRoundTrips) {
    const auto bytes = serializeClientHello(42U);
    const auto result = deserializeClientHello(bytes);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sequence, 42U);
    EXPECT_EQ(readPacketType(bytes), PacketType::ClientHello);
}

TEST(SerializationTest, ClientInputRoundTrips) {
    ClientInputCommand input;
    input.sequence = 17U;
    input.clientTick = 123456789U;
    input.movement = {-0.5f, 1.0f};
    input.lookDelta = {3.25f, -7.5f};

    const auto result = deserializeClientInput(serializeClientInput(input));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sequence, input.sequence);
    EXPECT_EQ(result->clientTick, input.clientTick);
    EXPECT_EQ(result->movement, input.movement);
    EXPECT_EQ(result->lookDelta, input.lookDelta);
}

TEST(SerializationTest, DisconnectRoundTrips) {
    const auto result = deserializeDisconnect(serializeDisconnect(91U));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sequence, 91U);
}

TEST(SerializationTest, ServerWelcomeRoundTrips) {
    const auto bytes = serializeServerWelcome(8U, 99U);
    const auto result = deserializeServerWelcome(bytes);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sequence, 8U);
    EXPECT_EQ(result->playerId, 99U);
    EXPECT_EQ(readPacketType(bytes), PacketType::ServerWelcome);
}

TEST(SerializationTest, ServerSnapshotRoundTrips) {
    ServerSnapshot snapshot;
    snapshot.sequence = 31U;
    snapshot.serverTick = 987654321U;
    snapshot.player.playerId = 7U;
    snapshot.player.position = {10.5f, 1.8f, -4.25f};
    snapshot.player.yawRadians = 0.75f;
    snapshot.player.pitchRadians = -0.25f;

    const auto result = deserializeServerSnapshot(serializeServerSnapshot(snapshot));

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->sequence, snapshot.sequence);
    EXPECT_EQ(result->serverTick, snapshot.serverTick);
    EXPECT_EQ(result->player.playerId, snapshot.player.playerId);
    EXPECT_EQ(result->player.position, snapshot.player.position);
    EXPECT_FLOAT_EQ(result->player.yawRadians, snapshot.player.yawRadians);
    EXPECT_FLOAT_EQ(result->player.pitchRadians, snapshot.player.pitchRadians);
}

TEST(SerializationTest, TruncatedPacketsAreRejected) {
    ClientInputCommand input;
    ServerSnapshot snapshot;

    expectEveryTruncationRejected(serializeClientHello(1U), deserializeClientHello);
    expectEveryTruncationRejected(serializeClientInput(input), deserializeClientInput);
    expectEveryTruncationRejected(serializeDisconnect(1U), deserializeDisconnect);
    expectEveryTruncationRejected(serializeServerWelcome(1U, 1U), deserializeServerWelcome);
    expectEveryTruncationRejected(serializeServerSnapshot(snapshot), deserializeServerSnapshot);
}

TEST(SerializationTest, WrongProtocolVersionIsRejected) {
    auto bytes = serializeClientInput({});
    std::fill_n(bytes.begin(), sizeof(protocolVersion), std::uint8_t{0});

    EXPECT_FALSE(readPacketType(bytes).has_value());
    EXPECT_FALSE(deserializeClientInput(bytes).has_value());
}

TEST(SerializationTest, WrongPacketTypeIsRejected) {
    EXPECT_FALSE(deserializeClientInput(serializeClientHello(1U)).has_value());
    EXPECT_FALSE(deserializeServerSnapshot(serializeServerWelcome(1U, 1U)).has_value());
}

} // namespace
} // namespace game
