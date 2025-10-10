#include "renderer.hpp"
#include "dataStructs/material.hpp"
#include "utilities/vec3.hpp"
#include <cstddef>

Color Renderer::rayColor(Ray const & ray, SceneSettings const & scene,  // NOLINT
                         ConfigSettings const & config, RandomGenerator materialRng) {
  if (ray.depth <= 0) {
    return {0.0F, 0.0F, 0.0F};
  }

  float closest_t = std::numeric_limits<float>::infinity();
  std::optional<HitRecord> hit_rec;
  size_t const num_spheres = scene.spheres.x.size();
  for (size_t i = 0; i < num_spheres; ++i) {
    if (auto new_hit = Renderer::RenderSpheres(scene, i, ray, closest_t)) {
      closest_t = new_hit->t;
      hit_rec   = new_hit;
    }
  }

  size_t const num_cylinders = scene.cylinders.x.size();
  for (size_t i = 0; i < num_cylinders; ++i) {
    if (auto new_hit = Renderer::RenderCylinders(scene, i, ray, closest_t)) {
      closest_t = new_hit->t;
      hit_rec   = new_hit;
    }
  }

  if (hit_rec) {
    MaterialID material_id = scene.materialTable[hit_rec->material_global_id];

    // Creamos el contexto de material usando punteros en lugar de referencias
    MaterialContext ctx(&scene, &config, &materialRng);

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
                                                           float closest_t) {
  // Extraemos los datos de la esfera 'i' de la estructura SoA
  Point3 sphere_center(scene.spheres.x[sphere_index], scene.spheres.y[sphere_index],
                       scene.spheres.z[sphere_index]);
  float sphere_radius = scene.spheres.r[sphere_index];

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
  if (root <= 0.001F or root >= closest_t) {
    root = (-half_b + sqrtd) / a;
    // Si la segunda raíz tampoco es válida, no hay colisión útil.
    if (root <= 0.001F or root >= closest_t) {
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

// Intersección de rayos con cilindros
std::optional<Renderer::HitRecord> Renderer::RenderCylinders(SceneSettings const & scene,  // NOLINT
                                                             size_t cylinder_index, Ray r,
                                                             float closest_t) {
  // Extraemos los datos de los cilindros de la escena
  Point3 centro_cilindro(scene.cylinders.x[cylinder_index], scene.cylinders.y[cylinder_index],
                         scene.cylinders.z[cylinder_index]);

  // Radio del cilindro
  float radio_cilindro = scene.cylinders.r[cylinder_index];

  // Altura del cilindro
  float h = Vec3(scene.cylinders.vx[cylinder_index], scene.cylinders.vy[cylinder_index],
                 scene.cylinders.vz[cylinder_index])
                .length();

  // Vector unitario del eje del cilindro â
  Vec3 eje_unitario = Vec3(scene.cylinders.vx[cylinder_index], scene.cylinders.vy[cylinder_index],
                           scene.cylinders.vz[cylinder_index])
                          .normalize();

  // Variables para trackear la intersección más cercana encontrada
  float interseccion_cercana = closest_t;
  Vec3 intersection_point;
  Vec3 intersection_normal;
  bool found_intersection = false;

  // 1: Intersección con la curva del cilindro

  // Calculamos el vector desde el origen del rayo hasta el centro del cilindro
  Vec3 oc = r.point - centro_cilindro;

  Vec3 dr_perp = component_perpendicular(r.direction, eje_unitario);  // dr_a
  Vec3 rc_perp = component_perpendicular(oc, eje_unitario);           // dc_a

  // Coeficientes de la ecuación cuadrática para superficie curva
  float a             = dot(dr_perp, dr_perp);
  float b             = 2.0F * dot(rc_perp, dr_perp);
  float c             = dot(rc_perp, rc_perp) - radio_cilindro * radio_cilindro;
  float discriminante = b * b - 4 * a * c;

  // para probar las soluciones a la ecuación acorde al caso
  std::vector<float> lambdas;

  if (discriminante < 0.0F) {
    // discriminante < 0 : No hay intersección por lo que no se hace nada
  } else if (std::fabs(discriminante) < 1e-12F) {
    // discriminante = 1 : Solo hay una intersección
    lambdas.push_back(-b / (2.0F * a));
  } else {
    // discriminante > 1 : Dos puntos de intersección
    lambdas.push_back((-b - std::sqrt(discriminante)) / (2.0F * a));
    lambdas.push_back((-b + std::sqrt(discriminante)) / (2.0F * a));
  }

  // Buscamos un punto de intersección valido
  for (float lambda : lambdas) {
    if (lambda > 0.001F and lambda < interseccion_cercana) {
      // heckeamos si el punto de intersección está comprendido en la altura del cilindro
      Point3 punto_potencial = r.at(lambda);
      float proyeccion       = dot(punto_potencial - centro_cilindro, eje_unitario);

      if (std::fabs(proyeccion) <= (h * 0.5F)) {
        interseccion_cercana = lambda;
        intersection_point   = punto_potencial;
        // vector normal del cilindro en el punto de intersección
        intersection_normal =
            component_perpendicular(punto_potencial - centro_cilindro, eje_unitario).normalize();
        found_intersection = true;
      }
    }
  }

  //  2. intersección con las bases del cilindro
  Point3 top_center = centro_cilindro + (h * 0.5F) * eje_unitario;

  // Vector desde el origen del rayo hasta el centro de la base
  Vec3 rp_top = top_center - r.point;

  if (std::fabs(dot(r.direction, eje_unitario)) > 1e-8F) {
    float dp_bs = dot(rp_top, eje_unitario) / dot(r.direction, eje_unitario);

    if (dp_bs > 0.001F and dp_bs < interseccion_cercana) {
      Point3 potential_point = r.at(dp_bs);

      // Verificamos si el punto de intersección está dentro de la base
      if ((potential_point - top_center).length() <= radio_cilindro) {
        interseccion_cercana = dp_bs;
        intersection_point   = potential_point;
        intersection_normal  = eje_unitario;
        found_intersection   = true;
      }
    }
  }

  // Calculamos el centro de la base inferior del cilindro
  Point3 bottom_center = centro_cilindro - (h * 0.5F) * eje_unitario;

  Vec3 rp_inf = bottom_center - r.point;

  if (std::fabs(dot(r.direction, -eje_unitario)) > 1e-8F) {
    float dp_bi = dot(rp_inf, -eje_unitario) / dot(r.direction, -eje_unitario);

    if (dp_bi > 0.001F and dp_bi < interseccion_cercana) {
      Point3 potential_point = r.at(dp_bi);

      // Verificamos si el punto de intersección está dentro de la base
      if ((potential_point - bottom_center).length() <= radio_cilindro) {
        interseccion_cercana = dp_bi;
        intersection_point   = potential_point;
        intersection_normal  = -eje_unitario;
        found_intersection   = true;
      }
    }
  }

  // Registro de intersección
  if (found_intersection) {
    HitRecord rec;
    rec.t        = interseccion_cercana;
    rec.p        = intersection_point;
    rec.prev_ray = r;
    rec.set_face_normal(r, intersection_normal);
    rec.material_global_id =
        static_cast<unsigned int>(scene.cylinders.materialIndex[cylinder_index]);
    return rec;
  }
  return std::nullopt;
}

Color Renderer::backgroundColor(Ray const & r, ConfigSettings const & config) {
  Vec3 unit_direction = r.direction.normalize();
  auto t_bg = 0.5F * (unit_direction.y + 1.0F);  // Mapea la altura del rayo a un valor entre 0 y 1

  // Mezcla lineal entre el color claro y oscuro del fondo
  return (1.0F - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}

Color Renderer::matteColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int matte_idx = material_id.localIndex;
  Color attenuation      = {ctx.scene->matte.r[matte_idx], ctx.scene->matte.g[matte_idx],
                            ctx.scene->matte.b[matte_idx]};

  Vec3 bounce_direction = hit_rec.normal + ctx.materialRng->get_vector_minus1_to_1();

  if (std::fabs(bounce_direction.x) < 1e-8F and
      std::fabs(bounce_direction.y) < 1e-8F and
      std::fabs(bounce_direction.z) < 1e-8F)
  {
    bounce_direction = hit_rec.normal;
  }

  Ray bounced_ray(hit_rec.p, bounce_direction, hit_rec.prev_ray.depth - 1);
  return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}

Color Renderer::metalColor(MaterialID material_id, MaterialContext const & ctx, HitRecord hit_rec) {
  unsigned int metal_idx = material_id.localIndex;
  Color attenuation      = {ctx.scene->metal.r[metal_idx], ctx.scene->metal.g[metal_idx],
                            ctx.scene->metal.b[metal_idx]};
  float diffusion_factor = ctx.scene->metal.diffusion[metal_idx];

  Vec3 reflected_dir = reflect(hit_rec.prev_ray.direction.normalize(), hit_rec.normal);
  Vec3 fuzz          = diffusion_factor * ctx.materialRng->get_unit_sphere().normalize();
  Ray bounced_ray    = Ray(hit_rec.p, reflected_dir + fuzz, hit_rec.prev_ray.depth - 1);

  if (dot(bounced_ray.direction, hit_rec.normal) > 0) {
    return attenuation * rayColor(bounced_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
  }

  return {0.0F, 0.0F, 0.0F};
}

Color Renderer::refractiveColor(MaterialID material_id, MaterialContext const & ctx,
                                HitRecord hit_rec) {
  unsigned int refractive_idx = material_id.localIndex;
  float ior                   = ctx.scene->refractive.ior[refractive_idx];
  Vec3 unit_direction         = hit_rec.prev_ray.direction.normalize();

  float refraction_ratio = ior;
  if (!hit_rec.front_face) {
    refraction_ratio = 1.0F / ior;
  }

  float cos_theta = std::min(dot(-unit_direction, hit_rec.normal), 1.0F);
  float sin_theta = std::sqrt(1.0F - cos_theta * cos_theta);

  Vec3 direction;

  if (refraction_ratio * sin_theta > 1.0F) {
    direction = reflect(unit_direction, hit_rec.normal);
  } else {
    Vec3 i    = refraction_ratio * (unit_direction + cos_theta * hit_rec.normal);
    Vec3 j    = -std::sqrt(std::fabs(1.0F - i.length_squared())) * hit_rec.normal;
    direction = i + j;
  }

  Ray refracted_ray(hit_rec.p, direction, hit_rec.prev_ray.depth - 1);
  Color attenuation(1.0F, 1.0F, 1.0F);

  return attenuation * rayColor(refracted_ray, *ctx.scene, *ctx.config, *ctx.materialRng);
}
