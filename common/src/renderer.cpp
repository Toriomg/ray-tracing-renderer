#include "renderer.hpp"
#include "dataStructs/material.hpp"
#include "utilities/vec3.hpp"
#include <cstddef>

Color Renderer::rayColor(Ray const & ray, SceneSettings const & scene,
                         ConfigSettings const & config, RandomGenerator & materialRng) {
  if (ray.depth <= 0) {
    return {0.0, 0.0, 0.0};
  }
  double closest_t = std::numeric_limits<double>::infinity();
  std::optional<HitRecord> hit_rec;
  size_t const num_spheres = scene.spheres.x.size();  // Filtro AABB para esferas
  for (size_t i = 0; i < num_spheres; ++i) {
    if (AABB::intersect(ray, scene.spheres.aabbs[i], 0.001, closest_t))
    {  // comprobamos que está dentro de la caja, si no no hace falta calcular la intersección
      if (auto new_hit = Renderer::RenderSpheres(scene, i, ray, closest_t)) {
        closest_t = new_hit->t;
        hit_rec   = new_hit;
      }
    }
  }
  size_t const num_cylinders = scene.cylinders.x.size();  // gestion de AABB para cilindros
  for (size_t i = 0; i < num_cylinders; ++i)
  {  // comprobamos que está dentro de la caja y si no, no se calcula la intersección
    if (AABB::intersect(ray, scene.cylinders.aabbs[i], 0.001, closest_t)) {
      if (auto new_hit = Renderer::RenderCylinders(scene, i, ray, closest_t)) {
        closest_t = new_hit->t;
        hit_rec   = new_hit;
      }
    }
  }
  if (hit_rec) {
    MaterialID material_id = scene.materialTable[hit_rec->material_global_id];
    MaterialContext ctx(
        &scene, &config,
        &materialRng);  // Creamos el contexto de material usando punteros en lugar de referencias

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
                                                           size_t sphere_index, Ray r,
                                                           double closest_t) {
  // Extraemos los datos de la esfera 'i' de la estructura SoA
  Point3 sphere_center(scene.spheres.x[sphere_index], scene.spheres.y[sphere_index],
                       scene.spheres.z[sphere_index]);
  double sphere_radius = scene.spheres.r[sphere_index];

  // ----- Matemática de la intersección Rayo-Esfera -----
  Vec3 oc           = r.point - sphere_center;
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
  rec.t               = root;
  rec.p               = r.at(root);
  rec.prev_ray        = r;
  Vec3 outward_normal = (rec.p - sphere_center) / sphere_radius;
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

std::optional<Renderer::Intersection> Renderer::intersectLateralSurface(  // NOLINT
    Ray const & r, CylinderGeometry const & cyl, double closest_t) {
  Vec3 const oc      = r.point - cyl.center;
  Vec3 const dr_perp = component_perpendicular(r.direction, cyl.unit_axis);
  Vec3 const rc_perp = component_perpendicular(oc, cyl.unit_axis);
  double const a     = dr_perp.length_squared();

  if (std::fabs(a) < 1e-8) {  // Evitar división por cero si el rayo es paralelo al eje.
    return std::nullopt;
  }

  double const b     = 2.0 * dot(rc_perp, dr_perp);
  double const c     = rc_perp.length_squared() - cyl.radius * cyl.radius;
  double const discr = b * b - 4 * a * c;

  if (discr < 0) {
    return std::nullopt;
  }

  // --- Lógica corregida para comprobar AMBAS raíces ---
  double const sqrt_discr  = std::sqrt(discr);
  double const half_height = cyl.height * 0.5;
  std::optional<Intersection> best_hit;

  // 1. Evaluar la primera raíz (la más cercana al origen del rayo)
  double t1 = (-b - sqrt_discr) / (2.0 * a);
  if (t1 > 0.001 and t1 < closest_t) {
    Point3 const p1          = r.at(t1);
    double const projection1 = dot(p1 - cyl.center, cyl.unit_axis);

    // Comprobamos si esta intersección está dentro de las tapas del cilindro
    if (std::fabs(projection1) <= half_height) {
      // Si es válida, la guardamos como nuestra mejor candidata hasta ahora.
      Vec3 normal = component_perpendicular(p1 - cyl.center, cyl.unit_axis).normalize();
      best_hit    = Intersection{t1, p1, normal};
    }
  }

  // 2. Evaluar la segunda raíz
  double t2 = (-b + sqrt_discr) / (2.0 * a);

  // Determinamos la distancia más cercana actual para no evaluar innecesariamente
  double current_closest = best_hit ? best_hit->t : closest_t;

  if (t2 > 0.001 and t2 < current_closest) {
    Point3 const p2          = r.at(t2);
    double const projection2 = dot(p2 - cyl.center, cyl.unit_axis);

    // Comprobamos si esta intersección está dentro de las tapas del cilindro
    if (std::fabs(projection2) <= half_height) {
      // Si es válida Y más cercana que la anterior, la guardamos.
      Vec3 normal = component_perpendicular(p2 - cyl.center, cyl.unit_axis).normalize();
      best_hit    = Intersection{t2, p2, normal};
    }
  }

  return best_hit;  // Devolvemos la mejor intersección encontrada (o nullopt si ninguna fue válida)
  // --- FIN DE LA LÓGICA CORREGIDA ---
}

void Renderer::updateBestHit(std::optional<Intersection> & best, double & closest,
                             std::optional<Intersection> const & new_hit) {
  if (new_hit and new_hit->t < closest) {
    best    = new_hit;
    closest = new_hit->t;
  }
}

std::optional<Renderer::HitRecord> Renderer::RenderCylinders(SceneSettings const & scene,  // NOLINT
                                                             size_t idx, Ray r, double closest_t) {
  // --- 1. Setup - Using precomputed values from CylinderData ---
  Vec3 const raw_axis = {scene.cylinders.vx[idx], scene.cylinders.vy[idx], scene.cylinders.vz[idx]};

  // Use the precomputed inverse axis length to avoid sqrt operations
  double const inv_len = scene.cylinders.invAxisLen[idx];

  CylinderGeometry const cyl = {
    .center    = {scene.cylinders.x[idx], scene.cylinders.y[idx], scene.cylinders.z[idx]},
    .unit_axis = raw_axis * inv_len, // Multiply by inverse length instead of normalize()
    .radius    = scene.cylinders.r[idx],
    .height    = 1.0 / inv_len  // Height = 1 / invAxisLen (since invAxisLen = 1/length)
  };
  // --- 2. Lateral surface intersection ---
  double local_closest = closest_t;
  std::optional<Intersection> best_hit;
  updateBestHit(best_hit, local_closest, intersectLateralSurface(r, cyl, closest_t));

  // --- 3. Cap intersections ---
  double const radius_sq   = cyl.radius * cyl.radius;
  double const half_height = cyl.height * 0.5;
  updateBestHit(
      best_hit, local_closest,
      intersectCap(r, cyl.center + cyl.unit_axis * half_height, cyl.unit_axis, radius_sq));
  updateBestHit(
      best_hit, local_closest,
      intersectCap(r, cyl.center - cyl.unit_axis * half_height, -cyl.unit_axis, radius_sq));

  if (!best_hit) {
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

Color Renderer::backgroundColor(Ray const & r, ConfigSettings const & config) {
  Vec3 unit_direction = r.direction.normalize();
  auto t_bg = 0.5 * (unit_direction.y + 1.0);  // Mapea la altura del rayo a un valor entre 0 y 1

  // Mezcla lineal entre el color claro y oscuro del fondo
  return (1.0 - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}

Color Renderer::matteColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int matte_idx = material_id.localIndex;
  Color attenuation      = {ctx.scene->matte.r[matte_idx], ctx.scene->matte.g[matte_idx],
                            ctx.scene->matte.b[matte_idx]};

  Vec3 bounce_direction = hit_rec.normal + ctx.materialRng->get_vector_minus1_to_1();

  if (bounce_direction.is_near_zero()) {
    bounce_direction = hit_rec.normal;
  }

  Ray bounced_ray(hit_rec.p, bounce_direction, hit_rec.prev_ray.depth - 1);  // Creación nuevo rayo
  return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}

Color Renderer::metalColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int metal_idx  = material_id.localIndex;
  Color attenuation       = {ctx.scene->metal.r[metal_idx], ctx.scene->metal.g[metal_idx],
                             ctx.scene->metal.b[metal_idx]};
  double diffusion_factor = ctx.scene->metal.diffusion[metal_idx];

  Vec3 reflected_dir       = reflect(hit_rec.prev_ray.direction, hit_rec.normal);
  Vec3 fuzz                = diffusion_factor * ctx.materialRng->get_vector_minus1_to_1();
  Vec3 scattered_direction = reflected_dir.normalize() + fuzz;

  Ray bounced_ray = Ray(hit_rec.p, scattered_direction, hit_rec.prev_ray.depth - 1);

  return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}

Color Renderer::refractiveColor(MaterialID material_id, MaterialContext const & ctx,
                                HitRecord hit_rec) {
  unsigned int refractive_idx = material_id.localIndex;
  double ior                  = ctx.scene->refractive.ior[refractive_idx];
  Vec3 unit_direction         = hit_rec.prev_ray.direction.normalize();

  double refraction_ratio = hit_rec.front_face ? (1.0 / ior) : ior;

  double cos_theta = std::min(-dot(unit_direction, hit_rec.normal), 1.0);
  double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

  Vec3 direction;

  if (refraction_ratio * sin_theta > 1.0) {
    direction = reflect(unit_direction, hit_rec.normal);  // Reflexión interna total
  } else {                                                // Refracción normal
    Vec3 i              = refraction_ratio * (unit_direction + cos_theta * hit_rec.normal);
    double discriminant = 1.0 - i.length_squared();
    Vec3 j              = -std::sqrt(std::max(0.0, discriminant)) * hit_rec.normal;
    direction           = i + j;
  }

  Ray refracted_ray(hit_rec.p, direction, hit_rec.prev_ray.depth - 1);
  Color attenuation(1.0, 1.0, 1.0);

  return attenuation * rayColor(refracted_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}
