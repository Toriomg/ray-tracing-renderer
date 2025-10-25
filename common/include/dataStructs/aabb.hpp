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
    // El inicializador {x:..., y:...} no es válido. Usa el constructor normal.
    Vec3 invDir = {1.0 / r.direction.x, 1.0 / r.direction.y, 1.0 / r.direction.z};

    // Usamos std::array en lugar de arrays de estilo C.
    // También usamos 'bool' para 'sign', es más claro y la conversión a 0/1 es segura.
    std::array<bool, 3> sign = {invDir.x < 0, invDir.y < 0, invDir.z < 0};

    // Usamos std::array para los límites de la caja.
    std::array<Point3, 2> bounds = {box.min_point, box.max_point};

    for (size_t axis = 0; axis < 3; ++axis) {
        
        // Esta lógica es óptima (sin saltos) y ahora no genera advertencias
        // porque 'axis' ya es del tipo correcto.
        double const t_near = ((sign.at(axis) ? bounds.at(1) : bounds.at(0))[axis] - r.point[axis]) * invDir[axis];
        double const t_far  = ((sign.at(axis) ? bounds.at(0) : bounds.at(1))[axis] - r.point[axis]) * invDir[axis];

        t_min = std::max(t_near, t_min);
        t_max = std::min(t_far, t_max);

        if (t_min > t_max) {
            return false;
        }
    }

    return true;
  }
};

#endif
