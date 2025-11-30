#include "renderer.hpp"
#include "dataStructs/bvh.hpp"
#include "dataStructs/hit_record.hpp"
#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"
#include "ray.hpp"
#include "utilities/random.hpp"
#include "utilities/vec3.hpp"
#include <../include/dataStructs/bvh.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>

Color Renderer::rayColor(Ray const & ray, SceneSettings const & scene,
                         ConfigSettings const & config, RandomGenerator & materialRng) {
  if (ray.depth <= 0) {
    return {0.0, 0.0, 0.0};
  }

  // 1. Objeto donde guardaremos el resultado final (si hay golpe)
  HitRecord rec;

  // 2. Preparamos el paquete de datos para la travesía
  // closest_t empieza en infinito porque aún no hemos golpeado nada.
  TraversalData trav_data{
    std::ref(ray),                           // Envolvemos const Ray&
    std::ref(scene),                         // Envolvemos const SceneSettings&
    std::ref(rec),                           // Envolvemos HitRecord& (mutable)
    0.001,                                   // t_min (evitar acne propio)
    std::numeric_limits<double>::infinity()  // closest_t inicial
  };

  // 3. Disparamos la búsqueda en el BVH
  // Si devuelve true, 'rec' contiene el objeto más cercano y 'trav_data.closest_t' la distancia.
  if (scene.bvh.hit(trav_data)) {
    // Recuperamos el material usando el ID que 'hit_sphere'/'hit_cylinder' escribieron en 'rec'
    MaterialID const material_id = scene.materialTable[rec.material_global_id];
    MaterialContext const ctx(&scene, &config, &materialRng);

    // Calculamos el color (esto no cambia respecto a tu código original)
    switch (material_id.type) {
      case MATTE:      return Renderer::matteColor(material_id, ctx, rec);
      case METAL:      return Renderer::metalColor(material_id, ctx, rec);
      case REFRACTIVE: return Renderer::refractiveColor(material_id, ctx, rec);
      default:         break;
    }
  }

  // Si bvh.hit devuelve false, pintamos el fondo
  return Renderer::backgroundColor(ray, config);
}

bool Renderer::hit_sphere(size_t index, TraversalData & data) {
  auto const & spheres = data.scene.get().spheres;
  Ray const & r        = data.ray.get();

  Point3 const center(spheres.x[index], spheres.y[index], spheres.z[index]);
  double const radius = spheres.r[index];

  Vec3 const oc             = r.point - center;
  double const a            = r.direction.length_squared();
  double const half_b       = dot(oc, r.direction);
  double const c            = oc.length_squared() - radius * radius;
  double const discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return false;
  };

  double const sqrtd = std::sqrt(discriminant);
  double root        = (-half_b - sqrtd) / a;
  // Comprobamos contra data.closest_t. Si la esfera está a 10m y closest_t es 5m,
  // data.closest_t descarta esta intersección inmediatamente.
  if (root <= data.t_min or root >= data.closest_t) {
    root = (-half_b + sqrtd) / a;
    if (root <= data.t_min or root >= data.closest_t) {
      return false;
    }
  }
  data.closest_t = root;  // <--- ESTO permite al BVH podar ramas futuras

  // Escribimos en el HitRecord compartido
  HitRecord & rec           = data.rec.get();
  rec.t                     = root;
  rec.p                     = r.at(root);
  Vec3 const outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.material_global_id = spheres.materialIndex[index];
  rec.prev_ray           = r;
  return true;
}

bool Renderer::hit_cylinder(size_t index, TraversalData & data) {
  auto const & cyls = data.scene.get().cylinders;
  Ray const & r     = data.ray.get();
  Point3 const center(cyls.x[index], cyls.y[index], cyls.z[index]);
  Vec3 const raw_axis(cyls.vx[index], cyls.vy[index], cyls.vz[index]);
  double const radius  = cyls.r[index];
  double const inv_len = cyls.invAxisLen[index];
  double const height  = 1.0 / inv_len;
  Vec3 const unit_axis = raw_axis * inv_len;  // Asumiendo que guardaste raw axis

  CylinderGeometry const geo{center, unit_axis, radius, height};
  double const radius_sq                     = radius * radius;
  double const half_height                   = height * 0.5;
  double t_limit                             = data.closest_t;
  std::optional<Intersection> best_local_hit = std::nullopt;

  auto lat_hit = intersectLateralSurface(r, geo, data.t_min, t_limit);
  if (lat_hit) {
    best_local_hit = lat_hit;  // t_limit ya fue actualizado dentro de intersectLateralSurface
  }

  Point3 const top_center = center + unit_axis * half_height;
  auto top_hit            = intersectCap(r, top_center, unit_axis, radius_sq);
  updateBestHit(best_local_hit, t_limit, top_hit, data.t_min);
  Point3 const bot_center = center - unit_axis * half_height;
  auto bot_hit            = intersectCap(r, bot_center, -unit_axis, radius_sq);
  updateBestHit(best_local_hit, t_limit, bot_hit, data.t_min);

  if (best_local_hit) {
    data.closest_t  = best_local_hit->t;  // Actualizamos Global
    HitRecord & rec = data.rec.get();
    rec.t           = best_local_hit->t;
    rec.p           = best_local_hit->p;
    rec.set_face_normal(r, best_local_hit->normal);
    rec.material_global_id = static_cast<unsigned int>(cyls.materialIndex[index]);
    rec.prev_ray           = r;

    return true;
  }
  return false;
}

std::optional<Renderer::Intersection> Renderer::intersectCap(Ray const & r, Point3 const & center,
                                                             Vec3 const & normal,
                                                             double radius_sq) {
  double const denominator = dot(r.direction, normal);
  if (std::fabs(denominator) < 1e-8) {
    return std::nullopt;
  }  // Rayo paralelo

  double const t = dot(center - r.point, normal) / denominator;
  if (t <= 0.001) {
    return std::nullopt;
  }  // Intersección detrás del rayo

  Point3 const p = r.at(t);
  if ((p - center).length_squared() > radius_sq) {
    return std::nullopt;
  }  // Fuera del radio

  return Intersection{t, p, normal};
}

void Renderer::updateBestHit(std::optional<Intersection> & best, double & closest_limit,
                             std::optional<Intersection> const & new_hit, double t_min) {
  if (new_hit) {
    // Solo nos interesa si está en el rango válido [t_min, closest_limit)
    if (new_hit->t > t_min and new_hit->t < closest_limit) {
      closest_limit = new_hit->t;  // <--- CULLING: Reducimos el límite localmente
      best          = new_hit;
    }
  }
}

std::optional<Renderer::Intersection> Renderer::intersectLateralSurface(
    Ray const & r, CylinderGeometry const & cyl, double t_min, double & t_max_limit) {
  double const radius_sq = cyl.radius * cyl.radius;
  Vec3 const oc          = r.point - cyl.center;  // intersección lateral
  Vec3 const dr_perp     = component_perpendicular(r.direction, cyl.unit_axis);
  Vec3 const oc_perp     = component_perpendicular(oc, cyl.unit_axis);
  double const a         = dr_perp.length_squared();
  if (std::fabs(a) < 1e-8) {
    return std::nullopt;
  }
  double const b     = 2.0 * dot(oc_perp, dr_perp);
  double const c     = oc_perp.length_squared() - radius_sq;
  double const discr = b * b - 4 * a * c;

  double const sqrt_discr = std::sqrt(discr);
  double const inv_2a     = 1.0 / (2.0 * a);
  double t                = (-b - sqrt_discr) * inv_2a;

  auto check_height = [&](double val_t) -> bool {
    if (val_t <= t_min or val_t >= t_max_limit) {
      return false;
    }
    Point3 const p           = r.at(val_t);
    double const height_proj = dot(p - cyl.center, cyl.unit_axis);
    return std::fabs(height_proj) <= (cyl.height * 0.5);
  };

  if (!check_height(t)) {
    t = (-b + sqrt_discr) * inv_2a;
    if (!check_height(t)) {
      return std::nullopt;
    }
  }

  t_max_limit       = t;
  Point3 const p    = r.at(t);
  Vec3 const normal = component_perpendicular(p - cyl.center, cyl.unit_axis).normalize();
  return Intersection{t, p, normal};
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------COLORES----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------------------------------------*/

Color Renderer::backgroundColor(Ray const & r, ConfigSettings const & config) {
  Vec3 const unit_direction = r.direction.normalize();
  auto t_bg = 0.5 * (unit_direction.y + 1.0);  // Mapea la altura del rayo a un valor entre 0 y 1

  // Mezcla lineal entre el color claro y oscuro del fondo
  return (1.0 - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}

Color Renderer::matteColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int const matte_idx = material_id.localIndex;
  Color const attenuation      = {ctx.scene->matte.r[matte_idx], ctx.scene->matte.g[matte_idx],
                                  ctx.scene->matte.b[matte_idx]};

  Vec3 bounce_direction = hit_rec.normal + ctx.materialRng->get_vector_minus1_to_1();

  if (bounce_direction.is_near_zero()) {
    bounce_direction = hit_rec.normal;
  }

  Ray const bounced_ray(hit_rec.p, bounce_direction,
                        hit_rec.prev_ray.depth - 1);  // Creación nuevo rayo
  return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}

Color Renderer::metalColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int const metal_idx  = material_id.localIndex;
  Color const attenuation       = {ctx.scene->metal.r[metal_idx], ctx.scene->metal.g[metal_idx],
                                   ctx.scene->metal.b[metal_idx]};
  double const diffusion_factor = ctx.scene->metal.diffusion[metal_idx];

  Vec3 const reflected_dir       = reflect(hit_rec.prev_ray.direction, hit_rec.normal);
  Vec3 const fuzz                = diffusion_factor * ctx.materialRng->get_vector_minus1_to_1();
  Vec3 const scattered_direction = reflected_dir.normalize() + fuzz;

  Ray const bounced_ray = Ray(hit_rec.p, scattered_direction, hit_rec.prev_ray.depth - 1);

  return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}

Color Renderer::refractiveColor(MaterialID material_id, MaterialContext const & ctx,
                                HitRecord hit_rec) {
  unsigned int const refractive_idx = material_id.localIndex;
  double const ior                  = ctx.scene->refractive.ior[refractive_idx];
  Vec3 const unit_direction         = hit_rec.prev_ray.direction.normalize();

  double const refraction_ratio = hit_rec.front_face ? (1.0 / ior) : ior;

  double const cos_theta = std::min(-dot(unit_direction, hit_rec.normal), 1.0);
  double const sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

  Vec3 direction;

  if (refraction_ratio * sin_theta > 1.0) {
    direction = reflect(unit_direction, hit_rec.normal);  // Reflexión interna total
  } else {                                                // Refracción normal
    Vec3 const i              = refraction_ratio * (unit_direction + cos_theta * hit_rec.normal);
    double const discriminant = 1.0 - i.length_squared();
    Vec3 const j              = -std::sqrt(std::max(0.0, discriminant)) * hit_rec.normal;
    direction                 = i + j;
  }

  Ray const refracted_ray(hit_rec.p, direction, hit_rec.prev_ray.depth - 1);
  Color const attenuation(1.0, 1.0, 1.0);

  return attenuation * rayColor(refracted_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}
