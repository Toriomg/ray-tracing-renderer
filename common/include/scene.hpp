#pragma once

#include "dataStructs/settings_structs.hpp"
#include "bvh.hpp"
#include <memory>

enum class PrimitiveType { Sphere, Cylinder };

struct PrimitiveInfo {
    AABB box;
    Point3 centroid; // Centro de la AABB, para construir la BVH
    PrimitiveType type;
    uint32_t original_index; // Índice en el array de Spheres o Cylinders
    PrimitiveInfo() = default;
};

class Scene {
    public:
    // Primitivos
    SphereData spheres;
    CylinderData cylinders;
    // Materiales
    std::vector<MaterialID> materialTable;

    // Estructuras de aceleración (BVH)
    std::vector<BVHNode> bvhNodes;
    std::vector<PrimitiveInfo> orderedPrimitives; // Las primitivas, reordenadas para que las hojas apunten a rangos contiguos

    Scene(std::shared_ptr<SceneSettings> sceneSetings);
    
    private:
    std::vector<PrimitiveInfo> buildPrimitiveInfo(
        const SphereData& spheres,
        const CylinderData& cylinders
    );

    // Función de ayuda recursiva para construir el árbol BVH
    uint32_t buildRecursive(std::vector<PrimitiveInfo>& primitiveInfos, uint32_t start, uint32_t end);
};