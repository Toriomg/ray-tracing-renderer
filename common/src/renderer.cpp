#include "renderer.hpp"
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

  double closest_t = std::numeric_limits<double>::infinity();
  std::optional<HitRecord> hit_rec;
  std::vector<BVHObject> intersected_objects;
  scene.bvh->get_intersected_objects(ray, 0.001, closest_t, intersected_objects);
  for (auto const & obj : intersected_objects) {
    std::optional<HitRecord> new_hit;

    switch (obj.type) {
      case BVHObject::SPHERE:
        new_hit = Renderer::RenderSpheres(scene, obj.index, ray, closest_t);
        break;
      case BVHObject::CYLINDER:
        new_hit = Renderer::RenderCylinders(scene, obj.index, ray, closest_t);
        break;
    }

    if (new_hit) {
      closest_t = new_hit->t;
      hit_rec   = new_hit;
    }
  }
  if (hit_rec) {
    MaterialID const material_id = scene.materialTable[hit_rec->material_global_id];
    MaterialContext const ctx(&scene, &config, &materialRng);

    switch (material_id.type) {
      case MATTE:      return Renderer::matteColor(material_id, ctx, *hit_rec);
      case METAL:      return Renderer::metalColor(material_id, ctx, *hit_rec);
      case REFRACTIVE: return Renderer::refractiveColor(material_id, ctx, *hit_rec);
      default:         break;
    }
  }
  return Renderer::backgroundColor(ray, config);
}

std::optional<Renderer::HitRecord> Renderer::RenderSpheres(SceneSettings const & scene,
                                                           size_t sphere_index, Ray const & r,
                                                           double closest_t) {
  // Extraemos los datos de la esfera 'i' de la estructura SoA
  Point3 const sphere_center(scene.spheres.x[sphere_index], scene.spheres.y[sphere_index],
                             scene.spheres.z[sphere_index]);
  double const sphere_radius = scene.spheres.r[sphere_index];

  // ----- Matemática de la intersección Rayo-Esfera -----
  Vec3 const oc     = r.point - sphere_center;
  auto a            = r.direction.length_squared();
  auto half_b       = dot(oc, r.direction);
  auto c            = oc.length_squared() - sphere_radius * sphere_radius;
  auto discriminant = half_b * half_b - a * c;

  // Si el discriminante es negativo, el rayo no toca la esfera. Pasamos a la siguiente.
  if (discriminant < 0) {
    return std::nullopt;
  }

  // Calculamos la raíz de la ecuación cuadrática para encontrar el punto de impacto 't'
  auto sqrtd = std::sqrt(discriminant);
  auto root  = (-half_b - sqrtd) / a;

  // Si la primera raíz no es válida (detrás del rayo o no es más cercana), prueba la segunda.
  if (root <= 0.001 or root >= closest_t) {
    root = (-half_b + sqrtd) / a;
    // Si la segunda raíz tampoco es válida, no hay colisión útil.
    if (root <= 0.001 or root >= closest_t) {
      return std::nullopt;
    }
  }
  // Hemos encontrado una colisión válida y más cercana. Llenamos el registro.
  HitRecord rec;
  rec.t                     = root;
  rec.p                     = r.at(root);
  rec.prev_ray              = r;
  Vec3 const outward_normal = (rec.p - sphere_center) / sphere_radius;
  rec.set_face_normal(r, outward_normal);
  rec.material_global_id = scene.spheres.materialIndex[sphere_index];
  return rec;
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

void Renderer::updateBestHit(std::optional<Intersection> & best, double & closest,
                             std::optional<Intersection> const & new_hit) {
  if (new_hit and new_hit->t < closest) {
    best    = new_hit;
    closest = new_hit->t;
  }
}

std::optional<Renderer::Intersection> Renderer::intersectLateralSurface(
    Ray const & r, CylinderGeometry const & cyl, double & t_max,
    std::optional<Intersection> & best_hit) {
  double const half_height = cyl.height * 0.5;
  double const radius_sq   = cyl.radius * cyl.radius;
  Vec3 const oc            = r.point - cyl.center;  // intersección lateral
  Vec3 const dr_perp       = component_perpendicular(r.direction, cyl.unit_axis);
  Vec3 const oc_perp       = component_perpendicular(oc, cyl.unit_axis);
  double const a           = dr_perp.length_squared();

  if (std::fabs(a) > 1e-8) {
    double const b     = 2.0 * dot(oc_perp, dr_perp);
    double const c     = oc_perp.length_squared() - radius_sq;
    double const discr = b * b - 4 * a * c;

    if (discr >= 0) {
      double const sqrt_discr = std::sqrt(discr);
      double const inv_2a     = 1.0 / (2.0 * a);
      double t                = (-b - sqrt_discr) * inv_2a;  // Raíz más cercana

      if (t > 0.001 and t < t_max) {
        Point3 const p = r.at(t);
        if (std::fabs(dot(p - cyl.center, cyl.unit_axis)) <= half_height) {
          Vec3 const normal = component_perpendicular(p - cyl.center, cyl.unit_axis);
          best_hit          = Intersection{t, p, normal};
          t_max             = t;
        }
      }
      if (!best_hit) {  // segunda raiz
        t = (-b + sqrt_discr) * inv_2a;
        if (t > 0.001 and t < t_max) {
          Point3 const p = r.at(t);
          if (std::fabs(dot(p - cyl.center, cyl.unit_axis)) <= half_height) {
            Vec3 const normal = component_perpendicular(p - cyl.center, cyl.unit_axis);
            best_hit          = Intersection{t, p, normal};
            t_max             = t;
          }
        }
      }
    }
  }
  return best_hit;
}

std::optional<Renderer::HitRecord> Renderer::RenderCylinders(SceneSettings const & scene,
                                                             size_t idx, Ray const & r,
                                                             double closest_t) {
  Point3 const center = {scene.cylinders.x[idx], scene.cylinders.y[idx], scene.cylinders.z[idx]};
  Vec3 const raw_axis = {scene.cylinders.vx[idx], scene.cylinders.vy[idx], scene.cylinders.vz[idx]};
  double const radius = scene.cylinders.r[idx];
  double const inv_len     = scene.cylinders.invAxisLen[idx];
  double const height      = 1.0 / inv_len;
  Vec3 const unit_axis     = raw_axis * inv_len;
  double const half_height = height * 0.5;
  double const radius_sq   = radius * radius;
  std::optional<Intersection> best_hit;
  double t_max = closest_t;

  CylinderGeometry const cyl_geo = {center, unit_axis, radius, height};  // intersección lateral
  intersectLateralSurface(r, cyl_geo, t_max, best_hit);

  Point3 const top_center = center + unit_axis * half_height;  // tapa superior
  if (auto cap_hit = intersectCap(r, top_center, unit_axis, radius_sq)) {
    if (cap_hit->t < t_max) {
      best_hit = cap_hit;
      t_max    = cap_hit->t;
    }
  }
  Point3 const bottom_center = center - unit_axis * half_height;  // Tapa inferior
  if (auto cap_hit = intersectCap(r, bottom_center, -unit_axis, radius_sq)) {
    if (cap_hit->t < t_max) {
      best_hit = cap_hit;
      t_max    = cap_hit->t;
    }
  }

  if (!best_hit) {  // hitrecord final
    return std::nullopt;
  }
  HitRecord rec;
  rec.t                  = best_hit->t;
  rec.p                  = best_hit->p;
  rec.prev_ray           = r;
  rec.material_global_id = static_cast<unsigned int>(scene.cylinders.materialIndex[idx]);
  rec.set_face_normal(r, best_hit->normal);
  return rec;
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
