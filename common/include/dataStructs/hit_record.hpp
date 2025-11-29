#ifndef HIT_RECORD_HPP
#define HIT_RECORD_HPP

#include "../../common/include/ray.hpp"
#include "utilities/vec3.hpp"

struct HitRecord {
  Point3 p;
  Vec3 normal;
  double t = 0.0;
  double u = 0.0;
  double v = 0.0;
  bool front_face = false;
  unsigned int material_global_id = 0;
  Ray prev_ray;

  HitRecord() = default;

  void set_face_normal(Ray const & r, Vec3 const & outward_normal) {
    front_face = dot(r.direction, outward_normal) < 0;
    normal     = front_face ? outward_normal : -outward_normal;
  }
};

#endif