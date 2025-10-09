#pragma once
#include "../../common/include/ray.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/random.hpp"


class Renderer{
    public:
    static Color rayColor(Ray const & r, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng);
    private:

    struct HitRecord {
        Point3 p; // Punto de colisión
        Vec3 normal; // Vector normal en el punto de colisión
        float t = 0.0F; // Parámetro 't' del rayo
        Ray prev_ray;
        unsigned int material_global_id = 0; // ID del material del objeto golpeado
        bool front_face = false; // Para saber si el rayo golpeó desde fuera o desde dentro

        HitRecord() = default;
        // Función para establecer la normal siempre apuntando hacia fuera
        void set_face_normal(const Ray& r, const Vec3& outward_normal) {
            bool front_face = dot(r.direction, outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }    // Índice del objeto golpeado en el array de la escena.
    };
    static std::optional<HitRecord> RenderSpheres(SceneSettings const & scene, size_t sphere_index, Ray r, float closest_t);
    static Color matteColor(MaterialID material_id, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng, HitRecord hit_rec);
    static Color metalColor(MaterialID material_id, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng, HitRecord hit_rec);
    static Color refractiveColor(MaterialID material_id, SceneSettings const & scene, ConfigSettings const & config, RandomGenerator materialRng, HitRecord hit_rec);
    static Color backgroundColor(Ray const & r, ConfigSettings const & config);
};
