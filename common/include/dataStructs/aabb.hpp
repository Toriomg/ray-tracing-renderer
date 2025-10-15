#ifndef AABB_HPP
#define AABB_HPP

#include "../utilities/vec3.hpp"
class Ray;

struct AABB {
  Point3 min;
  Point3 max;

  AABB() = default;

  AABB(Point3 const & a, Point3 const & b) : min(a), max(b) { }

  // Función para comprobar si un rayo intersecta la caja
  [[nodiscard]] bool hit(Ray const & r, double t_min, double t_max) const;
};

// Función para crear una caja que engloba a otras dos
AABB surrounding_box(const AABB & box0, const AABB & box1);

#endif
