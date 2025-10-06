#include "ray.hpp"
#include <optional> // For std::optional
#include <cmath> 

Color rayColor(const Ray& r, const SceneSettings& scene, const ConfigSettings& config) {// NOLINT
    // --- Variables para rastrear la colisión más cercana ---
    float closest_t = std::numeric_limits<float>::infinity(); // intersección más cercana
    std::optional<size_t> hit_sphere_index; // -1 significa que no hemos golpeado ninguna esfera todavía

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

        // Comprobamos si la colisión es válida (delante de la cámara y más cerca que las anteriores)
        // El umbral 0.001f evita problemas de precisión.
        if (root > 0.001F and root < closest_t) {
            closest_t = root;
            hit_sphere_index = i; // Store the hit record.
        }
    }

    // --- Después del bucle, decidimos qué color devolver ---

    // 1. Si golpeamos una esfera (el índice ya no es -1)
    if (hit_sphere_index != -1) {
        // Obtenemos el ID del material de la esfera que golpeamos
        unsigned int material_global_id = scene.spheres.materialIndex[*hit_sphere_index];
        
        // Usamos el ID para encontrar el tipo de material y su índice específico
        MaterialID material_id = scene.materialTable[material_global_id];

        // Por ahora, solo nos importa si es MATE
        if (material_id.type == MaterialType::MATTE) {
            unsigned int matte_idx = material_id.localIndex;
            
            // Devolvemos el color de ese material mate
            return {scene.matte.r[matte_idx], 
                         scene.matte.g[matte_idx], 
                         scene.matte.b[matte_idx]};
        }
    }

    // 2. Si no golpeamos ninguna esfera, devolvemos el color de fondo degradado
    Vec3 unit_direction = r.direction.normalize();
    auto t_bg = 0.5F * (unit_direction.y + 1.0F); // Mapea la altura del rayo a un valor entre 0 y 1
    
    // Mezcla lineal entre el color claro y oscuro del fondo
    return (1.0F - t_bg) * config.background_light_color + t_bg * config.background_dark_color;
}