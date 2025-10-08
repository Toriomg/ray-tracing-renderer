#include "dataStructs/material.hpp"
#include "renderer.hpp"
#include "utilities/vec3.hpp"
#include <cstddef>

Color Renderer::rayColor(Ray const & ray, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng) {
    if (ray.depth <= 0) {
        return {0.0F, 0.0F, 0.0F};
    }
    
    float closest_t = std::numeric_limits<float>::infinity();
    std::optional<HitRecord> hit_rec;
    size_t const num_spheres = scene.spheres.x.size();
    for (size_t i = 0; i < num_spheres; ++i) {
        if(auto new_hit = Renderer::RenderSpheres(scene, i, ray, closest_t)) {
            closest_t = new_hit->t;
            hit_rec   = new_hit;  
        }
    }
    size_t const num_cylinders = scene.cylinders.x.size();
    for (size_t i = 0; i < num_cylinders; ++i) {
    }
    if (hit_rec) {
        MaterialID material_id = scene.materialTable[hit_rec->material_global_id];
        switch (material_id.type) {
            case MATTE:
                return Renderer::matteColor(material_id, scene, config, materialRng, *hit_rec);
            case METAL:
                return Renderer::metalColor(material_id, scene, config, materialRng, *hit_rec);
            case REFRACTIVE:
                return {1.0F,1.0F,1.0F};
            default:
                break;
        }
    }
        return Renderer::backgroundColor(ray, config);
}

std::optional<Renderer::HitRecord> Renderer::RenderSpheres(SceneSettings const & scene, size_t sphere_index, Ray r, float closest_t){
    // Extraemos los datos de la esfera 'i' de la estructura SoA
    Point3 sphere_center(scene.spheres.x[sphere_index], scene.spheres.y[sphere_index], scene.spheres.z[sphere_index]);
    float sphere_radius = scene.spheres.r[sphere_index];

    // ----- Matemática de la intersección Rayo-Esfera -----
    Vec3 oc                  = r.point - sphere_center;
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

Color Renderer::backgroundColor(Ray const & r, ConfigSettings const & config){
    Vec3 unit_direction = r.direction.normalize();
    auto t_bg = 0.5F * (unit_direction.y + 1.0F);  // Mapea la altura del rayo a un valor entre 0 y 1

    // Mezcla lineal entre el color claro y oscuro del fondo
    return (1.0F - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}

Color Renderer::matteColor(MaterialID material_id, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng, HitRecord hit_rec){//NOLINT meter una wrapper de variables
    unsigned int matte_idx = material_id.localIndex;
    Color attenuation = {scene.matte.r[matte_idx], scene.matte.g[matte_idx], scene.matte.b[matte_idx]};
    Vec3 bounce_direction = hit_rec.normal.normalize() + materialRng.get_vector_minus1_to_1();
    if (bounce_direction.is_near_zero()) {
      bounce_direction = hit_rec.normal.normalize();
    }
    Ray bounced_ray(hit_rec.p, bounce_direction, hit_rec.prev_ray.depth - 1);
    return attenuation *  rayColor(bounced_ray, scene, config, materialRng);
}

Color Renderer::metalColor(MaterialID material_id, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng, HitRecord hit_rec){//NOLINT
    unsigned int metal_idx = material_id.localIndex;
    Color attenuation = {scene.metal.r[metal_idx], scene.metal.g[metal_idx], scene.metal.b[metal_idx]};
    float diffusion_factor = scene.metal.diffusion[metal_idx];

    // 1. Calcular dirección de reflejo perfecto
    Vec3 reflected_dir = reflect(hit_rec.prev_ray.direction.normalize(), hit_rec.normal);

    // 2. Añadir difusión (fuzziness)
    // Se multiplica por el factor de difusión para controlar la "borrosidad"
    Vec3 fuzz   = diffusion_factor * materialRng.get_unit_sphere().normalize();
    Ray bounced_ray = Ray(hit_rec.p, reflected_dir + fuzz, hit_rec.prev_ray.depth - 1);

    // 3. Si el rayo reflejado no se va "hacia afuera" de la superficie, se absorbe (color negro)
    // Esto evita que el rayo se refleje "hacia adentro" del objeto si la difusión es muy alta
    if (dot(bounced_ray.direction, hit_rec.normal) > 0) {
      return attenuation * rayColor(bounced_ray, scene, config, materialRng);
    }
    // El rayo fue absorbido
    return {0.0F, 0.0F, 0.0F};
}