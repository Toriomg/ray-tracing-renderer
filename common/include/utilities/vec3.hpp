#pragma once

#include <cmath>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <limits>

struct Vec3 {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;

    constexpr Vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}
    constexpr Vec3() noexcept = default;

    constexpr Vec3& operator+=(const Vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    constexpr Vec3& operator-=(const Vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    constexpr Vec3& operator*=(float scalar) noexcept {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr Vec3& operator/=(float scalar) noexcept {
        assert(scalar != 0.0f && "Division by zero!"); // this in release version is not compiled
        float inv_scalar =  (1.0F / scalar); // Multiplication is faster than division
        x *= inv_scalar;
        y *= inv_scalar;
        z *= inv_scalar;
        return *this;
    }
    
    // --- Unary Operator ---
    [[nodiscard]] constexpr Vec3 operator-() const noexcept {
        return {-x, -y, -z};
    }

    // --- Utility Functions ---
    // length_squared is much faster than length() as it avoids a square root.
    // Use it for comparisons whenever possible.
    [[nodiscard]] constexpr float length_squared() const noexcept {
        return x * x + y * y + z * z;
    }

    // length() is the magnitude of the vector.
    [[nodiscard]] float length() const noexcept {
        return std::sqrt(length_squared());
    }

    [[nodiscard]] Vec3 normalize() const noexcept {
        const float len_sq = length_squared();
        if (len_sq > std::numeric_limits<float>::epsilon()) {
            const float inv_len = 1.0F / std::sqrt(len_sq);
            return {x * inv_len, y * inv_len, z * inv_len};
        }
        return *this;
    }

    [[nodiscard]] constexpr bool is_near_zero() const noexcept {
        // A small value to avoid floating-point precision issues.
        constexpr auto s = 1e-8F;
        // Using a direct comparison is constexpr-friendly for all C++ versions.
        return (x > -s and x < s) and (y > -s and y < s) and (z > -s and z < s);
    }
};

// --- Type Aliases ---
// Define these after the struct but before their use.
using Color = Vec3;
using Point3 = Vec3;

[[nodiscard]] constexpr Vec3 operator+(const Vec3& u, const Vec3& v) noexcept {
    return {u.x + v.x, u.y + v.y, u.z + v.z};
}

[[nodiscard]] constexpr Vec3 operator-(const Vec3& u, const Vec3& v) noexcept {
    return {u.x - v.x, u.y - v.y, u.z - v.z};
}

// Component-wise multiplication (Hadamard product)
[[nodiscard]] constexpr Vec3 operator*(const Vec3& u, const Vec3& v) noexcept {
    return {u.x * v.x, u.y * v.y, u.z * v.z};
}

[[nodiscard]] constexpr Vec3 operator*(float scalar, const Vec3& v) noexcept {
    return {scalar * v.x, scalar * v.y, scalar * v.z};
}

[[nodiscard]] constexpr Vec3 operator*(const Vec3& v, float scalar) noexcept {
    return scalar * v; // Reuse the above operator
}

[[nodiscard]] constexpr Vec3 operator/(const Vec3& lhs, float rhs) {
    Vec3 result = lhs;
    result /= rhs;
    return result;
}

[[nodiscard]] inline Vec3 min(const Vec3& a, const Vec3& b) {
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

[[nodiscard]] inline Vec3 max(const Vec3& a, const Vec3& b) {
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}


[[nodiscard]] constexpr float dot(const Vec3& u, const Vec3& v) noexcept {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

[[nodiscard]] constexpr Vec3 cross(const Vec3& u, const Vec3& v) noexcept {
    return {u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x};
}

// --- Stream Output for Debugging ---
// This allows you to write `std::cout << my_vec;`
inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}