#pragma once

#include <cmath>

namespace game {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec3 operator+(Vec3 left, Vec3 right) {
    return {left.x + right.x, left.y + right.y, left.z + right.z};
}

inline Vec3 operator*(Vec3 value, float scalar) {
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}

inline float length(Vec2 value) {
    return std::sqrt((value.x * value.x) + (value.y * value.y));
}

inline Vec2 normalize(Vec2 value) {
    const float len = length(value);
    if (len <= 0.0001f) {
        return {};
    }
    return {value.x / len, value.y / len};
}

inline float clamp(float value, float minValue, float maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

} // namespace game
