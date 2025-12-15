#ifndef AABB_HPP
#define AABB_HPP

#include "utilities/vec3.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

struct Interval {
  double min;
  double max;

  Interval() : min(0.001), max(std::numeric_limits<double>::infinity()) { }

  Interval(double t_min, double t_max) : min(t_min), max(t_max) { }
};

class AABB {
public:
  Point3 min_point;
  Point3 max_point;

  AABB() {
    double const min_n = std::numeric_limits<double>::lowest();
    double const max_n = std::numeric_limits<double>::max();
    min_point          = {max_n, max_n, max_n};
    max_point          = {min_n, min_n, min_n};
  }

  AABB(Point3 const & a, Point3 const & b) : min_point(a), max_point(b) { }

  [[nodiscard]] bool intersect_fast(Point3 const & origin, Vec3 const & inv_dir, double t_min,
                                    double t_max_limit) const {
    double const tx1 = (min_point.x - origin.x) * inv_dir.x;
    double const tx2 = (max_point.x - origin.x) * inv_dir.x;

    // tmin empieza siendo el t_min que pasamos por parámetro (arregla el warning)
    double tmin = std::max(t_min, std::min(tx1, tx2));
    double tmax = std::min(t_max_limit, std::max(tx1, tx2));

    double const ty1 = (min_point.y - origin.y) * inv_dir.y;
    double const ty2 = (max_point.y - origin.y) * inv_dir.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    double const tz1 = (min_point.z - origin.z) * inv_dir.z;
    double const tz2 = (max_point.z - origin.z) * inv_dir.z;

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    return tmax >= tmin;
  }

  [[nodiscard]] static bool intersect(Point3 const & origin, Vec3 const & inv_d, AABB const & box,
                                      Interval const & t) {
    double t_min = t.min;
    double t_max = t.max;

    auto check = [&](double o, double inv, double b_min, double b_max) {
      double const t1 = (b_min - o) * inv;
      double const t2 = (b_max - o) * inv;
      t_min           = std::max(t_min, std::min(t1, t2));
      t_max           = std::min(t_max, std::max(t1, t2));
    };

    check(origin.x, inv_d.x, box.min_point.x, box.max_point.x);
    if (t_max <= t_min) {
      return false;
    }

    check(origin.y, inv_d.y, box.min_point.y, box.max_point.y);
    if (t_max <= t_min) {
      return false;
    }

    check(origin.z, inv_d.z, box.min_point.z, box.max_point.z);
    return t_max > t_min;
  }

  static AABB from_sphere(Point3 const & center, double radius) {
    Vec3 const r_vec(radius, radius, radius);
    return {center - r_vec, center + r_vec};
  }

  static AABB from_cylinder(Point3 const & center, Vec3 const & axis, double radius, double h) {
    Vec3 const half = axis.normalize() * (h * 0.5);
    Point3 const p1 = center - half;
    Point3 const p2 = center + half;
    Point3 const min_p(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
    Point3 const max_p(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
    Vec3 const r_vec(radius, radius, radius);
    return {min_p - r_vec, max_p + r_vec};
  }

  [[nodiscard]] Point3 centroid() const { return (min_point + max_point) * 0.5; }

  [[nodiscard]] int longest_axis() const {
    Vec3 const e = max_point - min_point;
    if (e.x > e.y and e.x > e.z) {
      return 0;
    }
    return (e.y > e.z) ? 1 : 2;
  }

  void expand_to_include(const AABB & other) {
    min_point.x = std::min(min_point.x, other.min_point.x);
    min_point.y = std::min(min_point.y, other.min_point.y);
    min_point.z = std::min(min_point.z, other.min_point.z);
    max_point.x = std::max(max_point.x, other.max_point.x);
    max_point.y = std::max(max_point.y, other.max_point.y);
    max_point.z = std::max(max_point.z, other.max_point.z);
  }
};
#endif
