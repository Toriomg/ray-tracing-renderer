#pragma once
#include "utilities/vec3.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/random.hpp"

struct Ray{
    Point3 point;
    Vec3 direction;

    constexpr Ray(Point3 point, Vec3 direction) noexcept : point(point), direction(direction) {}
    constexpr Ray() noexcept = default;
    [[nodiscard]] Point3 at(float t) const {
        return point + t * direction;
    }
};

struct HitRecord {
    Point3 p; // Punto de colisión
    Vec3 normal; // Vector normal en el punto de colisión
    float t = 0.0F; // Parámetro 't' del rayo
    unsigned int material_global_id = 0; // ID del material del objeto golpeado
    bool front_face = false; // Para saber si el rayo golpeó desde fuera o desde dentro

    HitRecord() = default;
    // Función para establecer la normal siempre apuntando hacia fuera
    void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }    // Índice del objeto golpeado en el array de la escena.
};

Color rayColor(const Ray& r, const SceneSettings& scene, const ConfigSettings& config, RandomGenerator materialRng, int depth);