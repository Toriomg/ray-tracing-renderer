#ifndef RAY_HPP
 #define RAY_HPP

 #include "dataStructs/settings_structs.hpp"
 #include "utilities/random.hpp"
 #include "utilities/vec3.hpp"

struct Ray {
  Point3 point;
  Vec3 direction;
  int depth = 0;

  constexpr Ray(Point3 point, Vec3 direction, int depth) noexcept
      : point(point), direction(direction), depth(depth) { }

  constexpr Ray() noexcept = default;

  [[nodiscard]] Point3 at(float t) const { return point + t * direction; }
};
