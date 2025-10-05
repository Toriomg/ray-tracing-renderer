#pragma once

#include "vec3.hpp"
class Ray;

struct AABB {
    Point3 min;
    Point3 max;

    AABB() = default;
    AABB(const Point3& a, const Point3& b) : min(a), max(b) {}
    // Función para comprobar si un rayo intersecta la caja
    bool hit(const Ray& r, float t_min, float t_max) const;
};

// Función para crear una caja que engloba a otras dos
AABB surrounding_box(const AABB& box0, const AABB& box1);