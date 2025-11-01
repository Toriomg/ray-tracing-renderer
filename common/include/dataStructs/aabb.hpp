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
    Point3 const min(center.x - radius, center.y - radius, center.z - radius);
    Point3 const max(center.x + radius, center.y + radius, center.z + radius);
    return AABB{min, max};
  }

  // Mismo constructor pero para cilindros (que no siempre están alineados con los ejes)
  static AABB from_cylinder(Point3 const & center, Vec3 const & axis, double radius,
                            double height) {
    Vec3 const unit_axis = axis.normalize();
    Vec3 const half_axis = 0.5 * height * unit_axis;
    Point3 const p1      = center - half_axis;
    Point3 const p2      = center + half_axis;

    Point3 const min(std::min(p1.x, p2.x) - radius, std::min(p1.y, p2.y) - radius,
                     std::min(p1.z, p2.z) - radius);
    Point3 const max(std::max(p1.x, p2.x) + radius, std::max(p1.y, p2.y) + radius,
                     std::max(p1.z, p2.z) + radius);
    return AABB{min, max};
  }

  [[nodiscard]] static bool intersect(Ray const & r, AABB const & box, double t_min, double t_max) {
    // Usando la implementación interna de Vec3 con el array e[3]
    Vec3 const invDir = {1.0 / r.direction.x, 1.0 / r.direction.y, 1.0 / r.direction.z};

    // Eje X
    double t0 = (box.min_point.x - r.point.x) * invDir.x;
    double t1 = (box.max_point.x - r.point.x) * invDir.x;
    if (invDir.x < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) {
      return false;
    }

    // Eje Y
    t0 = (box.min_point.y - r.point.y) * invDir.y;
    t1 = (box.max_point.y - r.point.y) * invDir.y;
    if (invDir.y < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) {
      return false;
    }

    // Eje Z
    t0 = (box.min_point.z - r.point.z) * invDir.z;
    t1 = (box.max_point.z - r.point.z) * invDir.z;
    if (invDir.z < 0.0) {
      std::swap(t0, t1);
    }
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    return t_max > t_min;
  }
};

#endif
