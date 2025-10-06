#pragma once
#include "vec3.hpp"
#include "dataStructs/settings_structs.hpp"

struct Ray{
    Point3 point;
    Vec3 direction;

    constexpr Ray(Point3 point, Vec3 direction) noexcept : point(point), direction(direction) {}
    constexpr Ray() noexcept = default;
};

struct HitRecord {
    float t;                    // Distancia a lo largo del rayo hasta el punto de impacto.
    size_t hit_object_index;    // Índice del objeto golpeado en el array de la escena.
};

Color rayColor(const Ray& r, const SceneSettings& scene, const ConfigSettings& config);