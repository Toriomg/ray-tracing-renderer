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


  [[nodiscard]] Point3 centroid() const {
        return Point3(
            (min_point.x + max_point.x) * 0.5,
            (min_point.y + max_point.y) * 0.5,
            (min_point.z + max_point.z) * 0.5
        );
    }

    // Find the longest axis (0=x, 1=y, 2=z)
    [[nodiscard]] int longest_axis() const {
        double dx = max_point.x - min_point.x;
        double dy = max_point.y - min_point.y;
        double dz = max_point.z - min_point.z;
        
        if (dx > dy && dx > dz) return 0;
        if (dy > dz) return 1;
        return 2;
    }

    // Calculate surface area of the AABB
    [[nodiscard]] double surface_area() const {
        double dx = max_point.x - min_point.x;
        double dy = max_point.y - min_point.y;
        double dz = max_point.z - min_point.z;
        return 2.0 * (dx * dy + dx * dz + dy * dz);
    }

    // Expand this AABB to include another AABB
    void expand_to_include(const AABB& other) {
        min_point.x = std::min(min_point.x, other.min_point.x);
        min_point.y = std::min(min_point.y, other.min_point.y);
        min_point.z = std::min(min_point.z, other.min_point.z);
        
        max_point.x = std::max(max_point.x, other.max_point.x);
        max_point.y = std::max(max_point.y, other.max_point.y);
        max_point.z = std::max(max_point.z, other.max_point.z);
    }

    // Combine two AABBs into one that contains both
    static AABB combine(const AABB& box1, const AABB& box2) {
        Point3 new_min(
            std::min(box1.min_point.x, box2.min_point.x),
            std::min(box1.min_point.y, box2.min_point.y),
            std::min(box1.min_point.z, box2.min_point.z)
        );
        Point3 new_max(
            std::max(box1.max_point.x, box2.max_point.x),
            std::max(box1.max_point.y, box2.max_point.y),
            std::max(box1.max_point.z, box2.max_point.z)
        );
        return AABB(new_min, new_max);
      }
};


#endif
