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
    Point3 min(center.x - radius, center.y - radius, center.z - radius);
    Point3 max(center.x + radius, center.y + radius, center.z + radius);
    return AABB{min, max};
  }

  // Mismo constructor pero para cilindros (que no siempre están alineados con los ejes)
  static AABB from_cylinder(Point3 const & center, Vec3 const & axis, double radius,
                            double height) {
    Vec3 unit_axis = axis.normalize();
    Vec3 half_axis = 0.5 * height * unit_axis;
    Point3 p1      = center - half_axis;
    Point3 p2      = center + half_axis;

    Point3 min(std::min(p1.x, p2.x) - radius, std::min(p1.y, p2.y) - radius,
               std::min(p1.z, p2.z) - radius);
    Point3 max(std::max(p1.x, p2.x) + radius, std::max(p1.y, p2.y) + radius,
               std::max(p1.z, p2.z) + radius);
    return AABB{min, max};
  }

  [[nodiscard]] static bool intersect(Ray const & r, AABB const & box, double t_min,
                                      double t_max) {  // intersección de caja AABB con rayos
    for (int axis = 0; axis < 3; ++axis) {
      double invD = 0.0;
      double t0   = 0.0;
      double t1   = 0.0;

      switch (axis) {  // stablecemos la intersección del rayo con cada eje de a caja AABB
        case 0:        // eje X
          invD = 1.0 / r.direction.x;
          t0   = (box.min_point.x - r.point.x) * invD;
          t1   = (box.max_point.x - r.point.x) * invD;
          break;
        case 1:  // eje Y
          invD = 1.0 / r.direction.y;
          t0   = (box.min_point.y - r.point.y) * invD;
          t1   = (box.max_point.y - r.point.y) * invD;
          break;
        case 2:  // eje Z
          invD = 1.0 / r.direction.z;
          t0   = (box.min_point.z - r.point.z) * invD;
          t1   = (box.max_point.z - r.point.z) * invD;
          break;
        default: continue;
      }

      if (invD < 0.0) {  // gestionamos el caso en el que la dirección del rayo es hacia atrás
        std::swap(t0, t1);
      }

      t_min = t0 > t_min ? t0 : t_min;  // entrada del rayo en toda la caja
      t_max = t1 < t_max ? t1 : t_max;  // primera salida del rayo de cualquiera de los ejes

      if (t_max <= t_min) {  // para que haya intersección el rayo debe haber entrado a la caja (no
                             // sale de ningún eje antes de haber entrado en todos)
        return false;
      }
    }
    return true;
  }
};

#endif
