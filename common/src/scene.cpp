#include "scene.hpp"
#include <algorithm>
#include <cstdint>
#include <numeric>
#include "constants.hpp"

AABB compute_bounds(const std::vector<PrimitiveInfo>& infos, uint32_t start, uint32_t end) {
    // Función auxiliar para calcular la caja que engloba a todas las primitivas en un rango
    if (start >= end) return AABB();
    
    AABB total_box = infos[start].box;
    for (uint32_t i = start + 1; i < end; ++i) {
        total_box = surrounding_box(total_box, infos[i].box);
    }
    return total_box;
}

Scene::Scene(std::shared_ptr<SceneSettings> sceneSettings){
    // Pasar de SceneSettings
    spheres = sceneSettings->spheres;
    cylinders = sceneSettings->cylinders;
    materialTable = sceneSettings->materialTable;

    // 2. Crear la lista unificada de información de primitivas
    std::vector<PrimitiveInfo> primitiveInfos = buildPrimitiveInfo(spheres, cylinders);

    // Si no hay primitivas, no hay nada que hacer
    if (primitiveInfos.empty()) {
        return;
    }

    // 3. Optimización: Pre-reservar memoria para los nodos.
    // Un árbol BVH tiene como máximo 2*N - 1 nodos para N primitivas.
    bvhNodes.reserve(primitiveInfos.size() * 2);

    // 4. Iniciar la construcción recursiva de la BVH
    buildRecursive(primitiveInfos, 0, primitiveInfos.size());

    // 5. Guardar las primitivas ya ordenadas
    this->orderedPrimitives = std::move(primitiveInfos);
}

uint32_t Scene::buildRecursive(std::vector<PrimitiveInfo>& primitiveInfos, uint32_t start, uint32_t end) {
    // --- Función Recursiva para construir la BVH ---
    // Reservar espacio para el nodo actual y obtener su índice.
    uint32_t currentNodeIndex = bvhNodes.size();
    bvhNodes.emplace_back(); 

    uint32_t numPrimitives = end - start;

    // --- Caso Base: Crear un nodo hoja ---
    if (numPrimitives <= Constants::MAX_PRIMS_IN_NODE) {
        BVHNode& node = bvhNodes[currentNodeIndex];
        node.first_primitive_offset = start;
        node.primitive_count = numPrimitives;
        node.box = compute_bounds(primitiveInfos, start, end);
        return currentNodeIndex;
    }

    // --- Caso Recursivo: Crear un nodo interno ---

    // 1. Calcular la caja de los centroides para elegir el eje de división
    Point3 centroid_min = primitiveInfos[start].centroid;
    Point3 centroid_max = primitiveInfos[start].centroid;
    for (uint32_t i = start + 1; i < end; ++i) {
        centroid_min = min(centroid_min, primitiveInfos[i].centroid);
        centroid_max = max(centroid_max, primitiveInfos[i].centroid);
    }
    Vec3 extent = centroid_max - centroid_min;

    // 2. Elegir el eje más largo
    int axis = 0;// Empezamos asumiendo X
    if (extent.y > extent.x) axis = 1; // El eje Y es más largo
    if (extent.z > (axis == 0 ? extent.x : extent.y)) {
        axis = 2; // El eje Z es el más largo de todos
    }

    uint32_t mid = start + numPrimitives / 2;

    // 3. Particionar las primitivas.
    // Usamos std::nth_element que es más rápido (promedio O(N)) que std::sort (O(N log N))
    // para encontrar el punto medio sin ordenar todo el rango.
    std::nth_element(
        primitiveInfos.begin() + start,
        primitiveInfos.begin() + mid,
        primitiveInfos.begin() + end,
        [axis](const PrimitiveInfo& a, const PrimitiveInfo& b) {
            switch (axis) {
                case 0: return a.centroid.x < b.centroid.x;
                case 1: return a.centroid.y < b.centroid.y;
                default: return a.centroid.z < b.centroid.z;
            }
        }
    );

    // 4. Llamar recursivamente para los dos subconjuntos
    uint32_t leftChildIndex = buildRecursive(primitiveInfos, start, mid);
    uint32_t rightChildIndex = buildRecursive(primitiveInfos, mid, end);

    // Debido a la naturaleza de la recursión, el nodo derecho siempre se creará
    // inmediatamente después del izquierdo en el vector bvhNodes.
    // assert(rightChildIndex == leftChildIndex + 1);

    // 5. Poblar el nodo interno actual
    BVHNode& node = bvhNodes[currentNodeIndex];
    node.first_primitive_offset = leftChildIndex;
    node.primitive_count = 0; // 0 indica que es un nodo interno
    node.box = surrounding_box(bvhNodes[leftChildIndex].box, bvhNodes[rightChildIndex].box);

    return currentNodeIndex;
}


std::vector<PrimitiveInfo> Scene::buildPrimitiveInfo(
    const SphereData& spheres,
    const CylinderData& cylinders
) {
    std::vector<PrimitiveInfo> infos;
    infos.reserve(spheres.x.size() + cylinders.x.size());

    // --- Spheres ---
    for (uint32_t i = 0; i < spheres.x.size(); i++) {
        float x = spheres.x[i];
        float y = spheres.y[i];
        float z = spheres.z[i];
        float r = spheres.r[i];

        Point3 min(x - r, y - r, z - r);
        Point3 max(x + r, y + r, z + r);
        Point3 centroid(x, y, z);

        infos.push_back({
            AABB(min, max),
            centroid,
            PrimitiveType::Sphere,
            i
        });
    }

    // --- Cylinders ---
    for (uint32_t i = 0; i < cylinders.x.size(); i++) {
        float x = cylinders.x[i];
        float y = cylinders.y[i];
        float z = cylinders.z[i];
        float r = cylinders.r[i];

        float vx = cylinders.vx[i];
        float vy = cylinders.vy[i];
        float vz = cylinders.vz[i];

        // Base y tope
        Point3 p0(x, y, z);
        Point3 p1(x + vx, y + vy, z + vz);

        Point3 min(
            std::min(p0.x, p1.x) - r,
            std::min(p0.y, p1.y) - r,
            std::min(p0.z, p1.z) - r
        );

        Point3 max(
            std::max(p0.x, p1.x) + r,
            std::max(p0.y, p1.y) + r,
            std::max(p0.z, p1.z) + r
        );

        Point3 centroid(
            0.5f * (p0.x + p1.x),
            0.5f * (p0.y + p1.y),
            0.5f * (p0.z + p1.z)
        );

        infos.push_back({
            AABB(min, max),
            centroid,
            PrimitiveType::Cylinder,
            i
        });
    }

    return infos;
}