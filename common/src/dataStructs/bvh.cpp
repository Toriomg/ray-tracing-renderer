#include "../include/dataStructs/bvh.hpp"
#include "../include/renderer.hpp"
#include "dataStructs/object.hpp"
#include "utilities/vec3.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

// Initializes the flat object list from separate Sphere and Cylinder arrays.
void BVH::init_build_data(SphereData const & s, CylinderData const & c) {
  nodes.clear();
  bvh_objects.clear();
  bvh_objects.reserve(s.x.size() + c.x.size());
  for (size_t i = 0; i < s.x.size(); ++i) {
    bvh_objects.push_back({BVHObject::SPHERE, i, s.aabbs.at(i)});
  }
  for (size_t i = 0; i < c.x.size(); ++i) {
    bvh_objects.push_back({BVHObject::CYLINDER, i, c.aabbs.at(i)});
  }
}

// Splits a node's objects into two halves based on the centroid median.
void BVH::partition_node(BuildTask const & task, int axis, std::vector<BuildTask> & stack) {
  size_t const mid = task.start + (task.end - task.start) / 2;

  // Comparator for sorting objects along the longest axis
  auto cmp = [axis](BVHObject const & a, BVHObject const & b) {
    return a.bbox.centroid()[static_cast<size_t>(axis)] <
           b.bbox.centroid()[static_cast<size_t>(axis)];
  };

  auto start_itr = bvh_objects.begin() + static_cast<std::ptrdiff_t>(task.start);
  auto mid_itr   = bvh_objects.begin() + static_cast<std::ptrdiff_t>(mid);
  auto end_itr   = bvh_objects.begin() + static_cast<std::ptrdiff_t>(task.end);

  // Partially sort so elements < mid are on the left (Linear time split)
  std::nth_element(start_itr, mid_itr, end_itr, cmp);

  size_t const left_idx = nodes.size();
  nodes.emplace_back();  // Create Left Child
  nodes.emplace_back();  // Create Right Child

  nodes.at(task.node_idx).left_first = static_cast<uint32_t>(left_idx);

  // Push children to stack (Right first, so Left is processed first)
  stack.push_back({left_idx + 1, mid, task.end});
  stack.push_back({left_idx, task.start, mid});
}

// Computes bounds and decides whether to make a leaf or split further.
void BVH::process_node(BuildTask const & task, std::vector<BuildTask> & stack) {
  AABB node_bounds;
  for (size_t i = task.start; i < task.end; ++i) {
    node_bounds.expand_to_include(bvh_objects.at(i).bbox);
  }
  nodes.at(task.node_idx).bounding_box = node_bounds;

  size_t const count = task.end - task.start;
  // Leaf condition: 4 or fewer primitives
  if (count <= 4) {
    nodes.at(task.node_idx).left_first = static_cast<uint32_t>(task.start);
    nodes.at(task.node_idx).prim_count = static_cast<uint16_t>(count);
    return;
  }

  // Internal node setup
  nodes.at(task.node_idx).prim_count = 0;
  int const axis                     = node_bounds.longest_axis();
  nodes.at(task.node_idx).axis       = static_cast<uint8_t>(axis);
  partition_node(task, axis, stack);
}

// Iteratively processes the build stack until all nodes are created.
void BVH::process_build_queue(std::vector<BuildTask> & stack) {
  while (!stack.empty()) {
    BuildTask const task = stack.back();
    stack.pop_back();
    process_node(task, stack);
  }
}

// Main entry point for building the BVH tree.
void BVH::build(SphereData const & spheres, CylinderData const & cylinders) {
  init_build_data(spheres, cylinders);
  if (bvh_objects.empty()) {
    return;
  }

  std::cout << "Building BVH: " << bvh_objects.size() << " objects.\n";
  nodes.reserve(bvh_objects.size() * 2);
  nodes.emplace_back();  // Create Root

  std::vector<BuildTask> stack;
  stack.reserve(64);
  stack.push_back({0, 0, bvh_objects.size()});
  process_build_queue(stack);

  std::cout << "BVH built: " << nodes.size() << " nodes.\n";
}

// Función auxiliar estática para procesar los objetos dentro de un nodo hoja.
// Devuelve true si alguno de los objetos fue golpeado.
namespace {

  bool process_leaf(LinearBVHNode const & node, std::vector<BVHObject> const & objects,
                    TraversalData & data) {
    bool hit_any = false;
    for (uint16_t i = 0; i < node.prim_count; ++i) {
      auto const & obj = objects[static_cast<size_t>(node.left_first) + i];

      bool hit = false;
      if (obj.type == BVHObject::SPHERE) {
        hit = Renderer::hit_sphere(obj.original_index, data);
      } else {
        hit = Renderer::hit_cylinder(obj.original_index, data);
      }

      if (hit) {
        hit_any = true;
      }
    }
    return hit_any;
  }

}  // namespace

bool BVH::hit(TraversalData & data) const {
  if (nodes.empty()) {
    return false;
  }
  Vec3 const inv_dir(1.0 / data.ray.get().direction.x, 1.0 / data.ray.get().direction.y,
                     1.0 / data.ray.get().direction.z);
  std::array<size_t, 64> stack{};
  size_t ptr        = 0;
  stack.at(ptr++)   = 0;  // Push root
  bool hit_anything = false;

  while (ptr > 0) {
    size_t const node_idx = stack.at(--ptr);  // Pop
    auto const & node     = nodes.at(node_idx);
    if (!node.bounding_box.intersect_fast(data.ray.get().point, inv_dir, data.t_min,
                                          data.closest_t))
    {
      continue;
    }
    if (node.prim_count > 0) {
      if (process_leaf(node, bvh_objects, data)) {
        hit_anything = true;
      }
      continue;
    }

    size_t const left_idx  = node.left_first;
    size_t const right_idx = left_idx + 1;

    bool const ray_neg = data.ray.get().direction[node.axis] < 0;
    if (ray_neg) {
      stack.at(ptr++) = left_idx;
      stack.at(ptr++) = right_idx;
    } else {
      stack.at(ptr++) = right_idx;
      stack.at(ptr++) = left_idx;
    }
  }

  return hit_anything;
}
