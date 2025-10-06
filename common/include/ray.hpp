#pragma once
#include "vec3.hpp"
#include "dataStructs/settings_structs.hpp"

struct Ray{
    Point3 point;
    Vec3 direction;

    constexpr Ray(Point3 point, Vec3 direction) noexcept : point(point), direction(direction) {}
    constexpr Ray() noexcept : point(), direction() {}
};

struct HitRecord {
    Point3 p;              // Punto de intersección
    Vec3   normal;         // Vector normal en la superficie en el punto p
    float  t;              // Parámetro 't' del rayo para el punto de colisión
    unsigned int object_index; // Índice del objeto golpeado (en el SoA)
    bool   front_face;     // Verdadero si el rayo golpea la cara exterior

    // Función para establecer la normal siempre apuntando contra el rayo
    inline void set_face_normal(const Ray& r, const Vec3& outward_normal) {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

Color rayColor(const Ray& r, const SceneSettings& scene);