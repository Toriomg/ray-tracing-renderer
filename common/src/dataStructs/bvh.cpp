// bvh.cpp
#include "../include/dataStructs/bvh.hpp"
#include <algorithm>

void BVH::build(SphereData const & spheres, CylinderData const & cylinders) {
  std::vector<BVHObject> all_objects;

  // Add spheres to BVH objects
  all_objects.reserve(spheres.x.size());
  for (size_t i = 0; i < spheres.x.size(); ++i) {
    all_objects.emplace_back(BVHObject::SPHERE, i, spheres.aabbs[i]);
  }

  // Add cylinders to BVH objects
  for (size_t i = 0; i < cylinders.x.size(); ++i) {
    all_objects.emplace_back(BVHObject::CYLINDER, i, cylinders.aabbs[i]);
  }

  total_objects = all_objects.size();

  std::cout << "Building BVH with " << total_objects << " objects (" << spheres.x.size()
            << " spheres, " << cylinders.x.size() << " cylinders)\n";

  // Reset statistics
  node_count = 0;
  leaf_count = 0;
  max_depth  = 0;

  // Build the tree recursively
  if (!all_objects.empty()) {
    root = build_recursive(all_objects, 0, static_cast<int>(all_objects.size()), 0);
  }

  std::cout << "BVH built: " << node_count << " nodes, " << leaf_count << " leaves, max depth "
            << max_depth << "\n";
}

std::shared_ptr<BVHNode> BVH::build_recursive(std::vector<BVHObject> & objects, int start, int end,
                                              int depth) {
  if (start >= end) {
    return nullptr;
  }

  node_count++;
  max_depth = std::max(max_depth, static_cast<size_t>(depth));
  auto node = std::make_shared<BVHNode>();

  AABB node_bounds;
  for (int i = start; i < end; ++i) {
    node_bounds.expand_to_include(objects[static_cast<size_t>(i)].bbox);
  }
  node->bounding_box = node_bounds;

  int object_count = end - start;
  if (object_count <= 4) {  // hoja con pocos objetos
    node->objects.assign(objects.begin() + start, objects.begin() + end);
    leaf_count++;
    return node;
  }

  int axis = node_bounds.longest_axis();
  std::sort(objects.begin() + start, objects.begin() + end,
            [axis](BVHObject const & a, BVHObject const & b) {
              return a.bbox.centroid()[static_cast<size_t>(axis)] <
                     b.bbox.centroid()[static_cast<size_t>(axis)];
            });  // dividir por la mediana para equilibrio

  int mid = start + object_count / 2;
  if (mid == start or mid == end) {  // ningun nodo puede estar vacio
    node->objects.assign(objects.begin() + start, objects.begin() + end);
    leaf_count++;
    return node;
  }

  node->left  = build_recursive(objects, start, mid, depth + 1);
  node->right = build_recursive(objects, mid, end, depth + 1);
  return node;
}

bool BVH::intersect(Ray const & ray, double t_min, double t_max) const {
  if (!root) {
    return false;
  }
  return intersect_recursive(root, ray, t_min, t_max);
}

bool BVH::intersect_recursive(std::shared_ptr<BVHNode> const & node, Ray const & ray, double t_min,
                              double t_max) const {
  if (!node) {
    return false;
  }

  if (!AABB::intersect(ray, node->bounding_box, t_min, t_max)) {
    return false;
  }

  if (node->is_leaf()) {
    return std::ranges::any_of(node->objects, [&](BVHObject const & obj) {
      return AABB::intersect(ray, obj.bbox, t_min, t_max);
    });
  }

  bool hit_left  = intersect_recursive(node->left, ray, t_min, t_max);
  bool hit_right = intersect_recursive(node->right, ray, t_min, t_max);

  return hit_left or hit_right;
}

void BVH::get_intersected_objects(Ray const & ray, double t_min, double t_max,
                                  std::vector<BVHObject> & result) const {
  if (!root) {
    return;
  }
  result.clear();
  IntersectionQuery query{ray, t_min, t_max, result};
  get_intersected_objects_recursive(root, query);
}

void BVH::get_intersected_objects_recursive(std::shared_ptr<BVHNode> const & node,
                                            IntersectionQuery & query) const {
  if (!node) {
    return;
  }

  if (!AABB::intersect(query.ray, node->bounding_box, query.t_min, query.t_max)) {
    return;
  }

  if (node->is_leaf()) {
    for (auto const & obj : node->objects) {
      query.result.push_back(obj);
    }
    return;
  }

  get_intersected_objects_recursive(node->left, query);
  get_intersected_objects_recursive(node->right, query);
}
