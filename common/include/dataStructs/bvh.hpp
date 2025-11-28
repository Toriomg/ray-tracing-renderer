// bvh.hpp
#ifndef BVH_HPP
#define BVH_HPP

#include "dataStructs/aabb.hpp"
#include "dataStructs/object.hpp"
#include "ray.hpp"
#include <memory>
#include <vector>

// objeto de tipo BVH
struct BVHObject {
  enum Type { SPHERE, CYLINDER };

  Type type;
  size_t index;  // indice dentro del array
  AABB bbox;

  BVHObject(Type t, size_t idx, const AABB & box) : type(t), index(idx), bbox(box) { }
};

// nodo BVH
struct BVHNode {
  AABB bounding_box;
  std::shared_ptr<BVHNode> left;
  std::shared_ptr<BVHNode> right;
  std::vector<BVHObject> objects;

  [[nodiscard]] bool is_leaf() const { return left == nullptr and right == nullptr; }

  BVHNode() = default;

  explicit BVHNode(const AABB & box) : bounding_box(box) { }
};

struct IntersectionQuery {
  Ray ray;
  double t_min;
  double t_max;
  std::vector<BVHObject> result;

  IntersectionQuery(Ray const & r, double tmin, double tmax, std::vector<BVHObject> & res)
      : ray(r), t_min(tmin), t_max(tmax), result(res) { }
};

class BVH {
public:
  BVH() = default;

  // creación de un arbol bvh en base a una escena
  void build(SphereData const & spheres, CylinderData const & cylinders);

  // busqueda de una interseccion en el arbol
  [[nodiscard]] bool intersect(Ray const & ray, double t_min, double t_max) const;

  // busqueda de todos los objetos con los que se produce una intersección
  void get_intersected_objects(Ray const & ray, double t_min, double t_max,
                               std::vector<BVHObject> & result) const;

  [[nodiscard]] size_t get_node_count() const { return node_count; }  // numero de nodos

  [[nodiscard]] size_t get_leaf_count() const { return leaf_count; }  // numero de hojas en el bvh

  [[nodiscard]] size_t get_max_depth() const { return max_depth; }  // profundidad del arbol

  [[nodiscard]] size_t get_object_count() const { return total_objects; }  // numero de objetos

private:
  std::shared_ptr<BVHNode> root;
  size_t node_count    = 0;
  size_t leaf_count    = 0;
  size_t max_depth     = 0;
  size_t total_objects = 0;

  // función de build recursiva
  std::shared_ptr<BVHNode> build_recursive(std::vector<BVHObject> & objects, int start, int end,
                                           int depth = 0);

  // función de intersección recursiva
  [[nodiscard]] bool intersect_recursive(std::shared_ptr<BVHNode> const & node, Ray const & ray,
                                         double t_min, double t_max) const;

  void get_intersected_objects_recursive(std::shared_ptr<BVHNode> const & node,
                                         IntersectionQuery & query) const;
};
#endif
