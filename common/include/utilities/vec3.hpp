#ifndef VEC3_HPP
#define VEC3_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

struct Vec3 {
  float x = 0.0F;
  float y = 0.0F;
  float z = 0.0F;

  constexpr Vec3(float x, float y, float z) noexcept : x(x), y(y), z(z) { }

  constexpr Vec3() noexcept = default;

  constexpr Vec3 & operator+=(Vec3 const & other) noexcept {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  constexpr Vec3 & operator-=(Vec3 const & other) noexcept {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  constexpr Vec3 & operator*=(float scalar) noexcept {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  constexpr Vec3 & operator/=(float scalar) noexcept {
    assert(scalar != 0.0F and "Division by zero!");  // this in release version is not compiled
    float inv_scalar = (1.0F / scalar);              // Multiplication is faster than division
    x *= inv_scalar;
    y *= inv_scalar;
    z *= inv_scalar;
    return *this;
  }

  // --- Unary Operator ---
  [[nodiscard]] constexpr Vec3 operator-() const noexcept { return {-x, -y, -z}; }

  // --- Utility Functions ---
  // length_squared is much faster than length() as it avoids a square root.
  // Use it for comparisons whenever possible.
  [[nodiscard]] constexpr float length_squared() const noexcept { return x * x + y * y + z * z; }

  // length() is the magnitude of the vector.
  [[nodiscard]] float length() const noexcept { return std::sqrt(length_squared()); }

  [[nodiscard]] Vec3 normalize() const noexcept {
    float const len_sq = length_squared();
    if (len_sq > std::numeric_limits<float>::epsilon()) {
      float const inv_len = 1.0F / std::sqrt(len_sq);
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
using Color  = Vec3;
using Point3 = Vec3;

[[nodiscard]] constexpr Vec3 operator+(Vec3 const & u, Vec3 const & v) noexcept {
  return {u.x + v.x, u.y + v.y, u.z + v.z};
}

[[nodiscard]] constexpr Vec3 operator-(Vec3 const & u, Vec3 const & v) noexcept {
  return {u.x - v.x, u.y - v.y, u.z - v.z};
}

// Component-wise multiplication (Hadamard product)
[[nodiscard]] constexpr Vec3 operator*(Vec3 const & u, Vec3 const & v) noexcept {
  return {u.x * v.x, u.y * v.y, u.z * v.z};
}

[[nodiscard]] constexpr Vec3 operator*(float scalar, Vec3 const & v) noexcept {
  return {scalar * v.x, scalar * v.y, scalar * v.z};
}

[[nodiscard]] constexpr Vec3 operator*(Vec3 const & v, float scalar) noexcept {
  return scalar * v;  // Reuse the above operator
}

[[nodiscard]] constexpr Vec3 operator/(Vec3 const & lhs, float rhs) {
  Vec3 result = lhs;
  result /= rhs;
  return result;
}

[[nodiscard]] inline Vec3 min(Vec3 const & a, Vec3 const & b) {
  return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

[[nodiscard]] inline Vec3 max(Vec3 const & a, Vec3 const & b) {
  return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

[[nodiscard]] constexpr float dot(Vec3 const & u, Vec3 const & v) noexcept {
  return u.x * v.x + u.y * v.y + u.z * v.z;
}

[[nodiscard]] constexpr Vec3 cross(Vec3 const & u, Vec3 const & v) noexcept {
  return {u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x};
}

[[nodiscard]] constexpr Vec3 reflect(Vec3 const & v, Vec3 const & n) noexcept {
  return v - 2 * dot(v, n) * n;
}

[[nodiscard]] constexpr Vec3 refract(Vec3 const & uv, Vec3 const & n,
                                     float etai_over_etat) noexcept {
  auto cos_theta      = std::min(dot(-uv, n), 1.0F);
  Vec3 r_out_perp     = etai_over_etat * (uv + cos_theta * n);
  Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0F - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

// --- Stream Output for Debugging ---
// This allows you to write `std::cout << my_vec;`
inline std::ostream & operator<<(std::ostream & os, Vec3 const & v) {
  return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
}

#endif
