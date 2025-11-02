#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../../common/include/ray.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/random.hpp"
#include <optional>

class Renderer {
public:
  struct MaterialContext {
    SceneSettings const * scene;    // Configuración de la escena
    ConfigSettings const * config;  // Configuración del renderizado
    RandomGenerator * materialRng;  // Generador de números aleatorios

    MaterialContext(SceneSettings const * s, ConfigSettings const * c, RandomGenerator * rng)
        : scene(s), config(c), materialRng(rng) { }
  };

  struct Intersection {  // Struct simple para intersección de cilindros
    double t = 0.0F;
    Point3 p;
    Vec3 normal;
  };

  static Color rayColor(Ray const & ray, SceneSettings const & scene, ConfigSettings const & config,
                        RandomGenerator & materialRng);

private:
  struct HitRecord {
    Point3 p;
    Vec3 normal;
    double t = 0.0F;
    Ray prev_ray;
    unsigned int material_global_id = 0;
    bool front_face                 = false;

    HitRecord() = default;

    void set_face_normal(Ray const & r, Vec3 const & outward_normal) {
      front_face = dot(r.direction, outward_normal) < 0;  // Actualizamos el miembro de la clase
      normal     = front_face ? outward_normal : -outward_normal;
    }
  };

  struct CylinderGeometry {
    Point3 center;
    Vec3 unit_axis;
    double radius = 0.0F;
    double height = 0.0F;
  };

  static std::optional<Intersection> intersectCap(Ray const & r, Point3 const & center,
                                                  Vec3 const & normal, double radius_sq);
  static std::optional<Intersection> intersectLateralSurface(
      Ray const & r, CylinderGeometry const & cyl, double & t_max,
      std::optional<Intersection> & best_hit);
  static void updateBestHit(std::optional<Intersection> & best, double & closest,
                            std::optional<Intersection> const & new_hit);

  static std::optional<HitRecord> RenderSpheres(SceneSettings const & scene, size_t sphere_index,
                                                Ray const & r, double closest_t);
  static std::optional<HitRecord> RenderCylinders(SceneSettings const & scene, size_t idx,
                                                  Ray const & r, double closest_t);

  static Color backgroundColor(Ray const & r, ConfigSettings const & config);

  static Color matteColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec);
  static Color metalColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec);
  static Color refractiveColor(MaterialID material_id, MaterialContext const & ctx,
                               HitRecord hit_rec);
};

#endif
