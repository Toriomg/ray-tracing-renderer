#ifndef BVH_HPP
#define BVH_HPP

#include "dataStructs/aabb.hpp"
#include "dataStructs/object.hpp"
#include "ray.hpp"
#include <cstdint>
#include <vector>

struct BVHObject {
  enum Type { SPHERE, CYLINDER };

  Type type             = SPHERE;
  size_t original_index = 0;
  AABB bbox;
};

struct alignas(8) LinearBVHNode {
  AABB bounding_box;
  uint32_t left_first = 0;
  uint16_t prim_count = 0;
  uint8_t axis        = 0;
  uint8_t pad         = 0;
};

// Defined here so it can be used in private member signatures
struct BuildTask {
  size_t node_idx;
  size_t start;
  size_t end;
};

class BVH {
public:
  BVH() = default;

  void build(SphereData const & spheres, CylinderData const & cylinders);

  [[nodiscard]] bool intersect(Ray const & ray, Interval const & t) const;

  void get_intersected_objects(Ray const & ray, Interval const & t,
                               std::vector<BVHObject> & result) const;

  [[nodiscard]] size_t get_node_count() const { return nodes.size(); }

  [[nodiscard]] size_t get_object_count() const { return bvh_objects.size(); }

private:
  std::vector<LinearBVHNode> nodes;
  std::vector<BVHObject> bvh_objects;

  // Helpers
  void init_build_data(SphereData const & s, CylinderData const & c);
  void process_build_queue(std::vector<BuildTask> & stack);
  void process_node(BuildTask const & task, std::vector<BuildTask> & stack);

  // Grouped task parameters to keep arg count <= 4
  void partition_node(BuildTask const & task, int axis, std::vector<BuildTask> & stack);

  [[nodiscard]] bool check_leaf(LinearBVHNode const & node, Point3 const & orig,
                                Vec3 const & inv_dir, Interval const & t) const;
};

#endif
