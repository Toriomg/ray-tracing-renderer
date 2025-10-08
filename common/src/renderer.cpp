#include "ray.hpp"
#include <optional> // For std::optional
#include <cmath> 

Color rayColor(const Ray& r, const SceneSettings& scene, const ConfigSettings& config, RandomGenerator materialRng, int depth) {// NOLINT
    if (depth <= 0) {
        return {0.0F,0.0F,0.0F};
    }
    // --- Variables para rastrear la colisión más cercana ---
    float closest_t = std::numeric_limits<float>::infinity(); // intersección más cercana
    std::optional<HitRecord> hit_rec;

    // --- Bucle principal: comprobar cada esfera de la escena ---
    const size_t num_spheres = scene.spheres.x.size();
    for (size_t i = 0; i < num_spheres; ++i) {
        
        // Extraemos los datos de la esfera 'i' de la estructura SoA
        Point3 sphere_center(scene.spheres.x[i], scene.spheres.y[i], scene.spheres.z[i]);
        float sphere_radius = scene.spheres.r[i];

        // ----- Matemática de la intersección Rayo-Esfera -----
        Vec3 oc = r.point - sphere_center;
        auto a = r.direction.length_squared();
        auto half_b = dot(oc, r.direction);
        auto c = oc.length_squared() - sphere_radius * sphere_radius;
        auto discriminant = half_b * half_b - a * c;

        // Si el discriminante es negativo, el rayo no toca la esfera. Pasamos a la siguiente.
        if (discriminant < 0) {
            continue;
        }
        
        // Calculamos la raíz de la ecuación cuadrática para encontrar el punto de impacto 't'
        auto sqrtd = std::sqrt(discriminant);
        auto root = (-half_b - sqrtd) / a;
        if (root <= 0.001F) { // Si esta raíz no es válida (está detrás o es demasiado cercana)
            // ...entonces probamos la segunda raíz (la del '+')
            root = (-half_b + sqrtd) / a;
            if (root <= 0.001F) { // Si esta tampoco es válida, no hay intersección útil
                continue; // Pasamos a la siguiente esfera
            }
        }
        // Comprobamos si la colisión es válida (delante de la cámara y más cerca que las anteriores)
        // El umbral 0.001f evita problemas de precisión.
        if (root < closest_t) {
            closest_t = root;
            // Llenamos el HitRecord con toda la información
            HitRecord temp_rec;
            temp_rec.t = root;
            temp_rec.p = r.at(root); // Calcula el punto de colisión
            Vec3 outward_normal = (temp_rec.p - sphere_center) / sphere_radius;
            temp_rec.set_face_normal(r, outward_normal); // Calcula la normal correcta
            temp_rec.material_global_id = scene.spheres.materialIndex[i];
            
            hit_rec = temp_rec; // Guardamos el registro de la colisión
        }
    }

    // --- Después del bucle, decidimos qué color devolver ---

    // 1. Si golpeamos una esfera (el índice ya no es -1)
    if (hit_rec) {
        // Obtenemos el ID del material de la esfera que golpeamos
        MaterialID material_id = scene.materialTable[hit_rec->material_global_id];

        // Por ahora, solo nos importa si es MATE
        if (material_id.type == MaterialType::MATTE) {
            unsigned int matte_idx = material_id.localIndex;
            Color material_color = {scene.matte.r[matte_idx], scene.matte.g[matte_idx], scene.matte.b[matte_idx]};

            Vec3 bounce_target = hit_rec->p + hit_rec->normal + materialRng.get_vector_minus1_to_1();
            if (bounce_target.is_near_zero()) {
                bounce_target = hit_rec->normal;
            }
            Ray bounced_ray(hit_rec->p, bounce_target - hit_rec->p);

            // Devolvemos el color de ese material mate
            return material_color * rayColor(bounced_ray, scene, config, materialRng, depth - 1);
        }
    }

    // 2. Si no golpeamos ninguna esfera, devolvemos el color de fondo degradado
    Vec3 unit_direction = r.direction.normalize();
    auto t_bg = 0.5F * (unit_direction.y + 1.0F); // Mapea la altura del rayo a un valor entre 0 y 1
    
    // Mezcla lineal entre el color claro y oscuro del fondo
    return (1.0F - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}