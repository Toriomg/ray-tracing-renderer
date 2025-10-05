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
    // Boundary Volume Hierarchy 
    std::vector<BVHNode> bvhNodes;
    
    Scene(std::shared_ptr<SceneSettings> sceneSetings);
    private:
    std::vector<PrimitiveInfo> buildPrimitiveInfo;
};