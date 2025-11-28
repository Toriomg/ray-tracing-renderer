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
    double min_n = std::numeric_limits<double>::lowest();
    double max_n = std::numeric_limits<double>::max();
    min_point    = {max_n, max_n, max_n};
    max_point    = {min_n, min_n, min_n};
  }

  AABB(Point3 const & a, Point3 const & b) : min_point(a), max_point(b) { }

  [[nodiscard]] static bool intersect(Point3 const & origin, Vec3 const & inv_d, AABB const & box,
                                      Interval const & t) {
    double t_min = t.min;
    double t_max = t.max;

    auto check = [&](double o, double inv, double b_min, double b_max) {
      double t1 = (b_min - o) * inv;
      double t2 = (b_max - o) * inv;
      t_min     = std::max(t_min, std::min(t1, t2));
      t_max     = std::min(t_max, std::max(t1, t2));
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
    Vec3 r_vec(radius, radius, radius);
    return {center - r_vec, center + r_vec};
  }

  static AABB from_cylinder(Point3 const & center, Vec3 const & axis, double radius, double h) {
    Vec3 half = axis.normalize() * (h * 0.5);
    Point3 p1 = center - half;
    Point3 p2 = center + half;
    Point3 min_p(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z));
    Point3 max_p(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z));
    Vec3 r_vec(radius, radius, radius);
    return {min_p - r_vec, max_p + r_vec};
  }

  [[nodiscard]] Point3 centroid() const { return (min_point + max_point) * 0.5; }

  [[nodiscard]] int longest_axis() const {
    Vec3 e = max_point - min_point;
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
