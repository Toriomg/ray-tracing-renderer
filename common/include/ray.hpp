#ifndef RAY_HPP
#define RAY_HPP

#include "dataStructs/settings_structs.hpp"
#include "utilities/random.hpp"
#include "utilities/vec3.hpp"

struct Ray {
  Point3 point;
  Vec3 direction;

  constexpr Ray(Point3 point, Vec3 direction) noexcept : point(point), direction(direction) { }

  constexpr Ray() noexcept = default;

  [[nodiscard]] Point3 at(float t) const { return point + t * direction; }
};

struct HitRecord {
  Point3 p;                                // Punto de colisión
  Vec3 normal;                             // Vector normal en el punto de colisión
  float t                         = 0.0F;  // Parámetro 't' del rayo
  unsigned int material_global_id = 0;     // ID del material del objeto golpeado
  bool front_face = false;  // Para saber si el rayo golpeó desde fuera o desde dentro

  HitRecord() = default;

  // Función para establecer la normal siempre apuntando hacia fuera
  void set_face_normal(Ray const & r, Vec3 const & outward_normal) {
    front_face = dot(r.direction, outward_normal) < 0;
    normal     = front_face ? outward_normal : -outward_normal;
  }  // Índice del objeto golpeado en el array de la escena.
};

Color rayColor(Ray const & r, SceneSettings const & scene, ConfigSettings const & config,
               RandomGenerator materialRng, int depth);

#endif
