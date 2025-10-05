#include "scene.hpp"
#include <algorithm>
#include <cstdint>

Scene::Scene(std::shared_ptr<SceneSettings> sceneSettings){
    // Pasar de SceneSettings
    spheres = sceneSettings->spheres;
    cylinders = sceneSettings->cylinders;
    materialTable = sceneSettings->materialTable;

    // Create the primitivesInfo
    for(uint32_t i = 0; i < spheres.r.size(); i++){
        // Calculate AABB
        AABB sphereAABB = {Point3(), Point3()};
        // Calcular centroide
        Point3 centroid = Point3();
        PrimitiveInfo SphereInfo = {
            sphereAABB,
            centroid,
            PrimitiveType::Sphere,
            i
        };
    }

    for(uint32_t i = 0; i < cylinders.r.size(); i++){
        // Calculate AABB
        AABB cylinderAABB = {Point3(), Point3()};
        // Calcular centroide
        Point3 centroid = Point3();
        PrimitiveInfo cylinderInfo = {
            cylinderAABB,
            centroid,
            PrimitiveType::Cylinder,
            i
        };
    }
}

