#pragma once

#include <cmath>
#include <iostream>
#include <cassert>

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) {}

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
        float inv_scalar =  (1.0f / scalar); // Multiplication is faster than division
        x /= inv_scalar;
        y /= inv_scalar;
        z /= inv_scalar;
        return *this;
    }
    
    // --- Unary Operator ---
    constexpr Vec3 operator-() const noexcept {
        return Vec3(-x, -y, -z);
    }

    // --- Utility Functions ---
    // length_squared is much faster than length() as it avoids a square root.
    // Use it for comparisons whenever possible.
    constexpr float length_squared() const noexcept {
        return x * x + y * y + z * z;
    }

    // length() is the magnitude of the vector.
    float length() const noexcept {
        return std::sqrt(length_squared());
    }

    Vec3 normalize() const noexcept {
        const float len_sq = length_squared();
        if (len_sq > std::numeric_limits<float>::epsilon()) {
            const float inv_len = 1.0f / std::sqrt(len_sq);
            // Multiplicamos el vector actual (*this) por el inverso de la longitud
            return (*this) * inv_len;
        }
        // Si la longitud es casi cero, devuelve el vector original sin cambios
        return *this;
    }
};

inline constexpr Vec3 operator+(const Vec3& u, const Vec3& v) noexcept {
    return Vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline constexpr Vec3 operator-(const Vec3& u, const Vec3& v) noexcept {
    return Vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

// Component-wise multiplication (Hadamard product)
inline constexpr Vec3 operator*(const Vec3& u, const Vec3& v) noexcept {
    return Vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline constexpr Vec3 operator*(float scalar, const Vec3& v) noexcept {
    return Vec3(scalar * v.x, scalar * v.y, scalar * v.z);
}

inline constexpr Vec3 operator*(const Vec3& v, float scalar) noexcept {
    return scalar * v; // Reuse the above operator
}

inline constexpr Vec3 operator/(const Vec3& v, float scalar) noexcept {
    assert(scalar != 0.0f && "Division by zero!");
    return v * (1.0f / scalar); // Multiplication is faster than division
}

using Color = Vec3;
using Point3 = Vec3;

// --- Non-Member Utility Functions ---

inline constexpr float dot(const Vec3& u, const Vec3& v) noexcept {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline constexpr Vec3 cross(const Vec3& u, const Vec3& v) noexcept {
    return Vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

// --- Stream Output for Debugging ---
// This allows you to write `std::cout << my_vec;`
inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}