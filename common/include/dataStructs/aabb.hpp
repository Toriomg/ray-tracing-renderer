// aabb.hpp
#ifndef AABB_HPP
#define AABB_HPP

#include "ray.hpp"
#include "utilities/vec3.hpp"
#include <algorithm>

class AABB {
public:
  Point3 min_point;
  Point3 max_point;

  AABB() = default;

  AABB(Point3 const & a, Point3 const & b) : min_point(a), max_point(b) { }

  // Contructor que permite meter una esfera en una caja AABB para simplificar las intersecciones
  static AABB from_sphere(Point3 const & center, double radius) {
    Point3 min(center.e[0] - radius, center.e[1] - radius, center.e[2] - radius);
    Point3 max(center.e[0] + radius, center.e[1] + radius, center.e[2] + radius);
    return AABB{min, max};
  }

  // Mismo constructor pero para cilindros (que no siempre están alineados con los ejes)
  static AABB from_cylinder(Point3 const & center, Vec3 const & axis, double radius,
                            double height) {
    Vec3 unit_axis = axis.normalize();
    Vec3 half_axis = 0.5 * height * unit_axis;
    Point3 p1      = center - half_axis;
    Point3 p2      = center + half_axis;

    Point3 min(std::min(p1.e[0], p2.e[0]) - radius, std::min(p1.e[1], p2.e[1]) - radius,
               std::min(p1.e[2], p2.e[2]) - radius);
    Point3 max(std::max(p1.e[0], p2.e[0]) + radius, std::max(p1.e[1], p2.e[1]) + radius,
               std::max(p1.e[2], p2.e[2]) + radius);
    return AABB{min, max};
  }

  [[nodiscard]] static bool intersect(Ray const & r, AABB const & box, double t_min, double t_max) {
    // Usando la implementación interna de Vec3 con el array e[3]
    Vec3 invDir = {1.0 / r.direction.e[0], 1.0 / r.direction.e[1], 1.0 / r.direction.e[2]};

    // Eje X
    double t0 = (box.min_point.e[0] - r.point.e[0]) * invDir.e[0];
    double t1 = (box.max_point.e[0] - r.point.e[0]) * invDir.e[0];
    if (invDir.e[0] < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) {
      return false;
    }

    // Eje Y
    t0 = (box.min_point.e[1] - r.point.e[1]) * invDir.e[1];
    t1 = (box.max_point.e[1] - r.point.e[1]) * invDir.e[1];
    if (invDir.e[1] < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) {
      return false;
    }

    // Eje Z
    t0 = (box.min_point.e[2] - r.point.e[2]) * invDir.e[2];
    t1 = (box.max_point.e[2] - r.point.e[2]) * invDir.e[2];
    if (invDir.e[2] < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    return t_max > t_min;
  }
};

#endif
