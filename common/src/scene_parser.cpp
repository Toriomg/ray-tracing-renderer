#include <vector>

// Materials structs

struct MatteMaterials {
    std::vector<float> r, g, b;
};

struct MetalMaterials {
    std::vector<float> r, g, b, diffusion;
};


struct RefractiveMaterials {
    std::vector<float> ior;
};

// Primitives structs

struct alignas(16) Sphere {
    float x, y, z, r;       // All the numbers that define the sphere
    int materialIndex;       // Index of its material
};

struct alignas(16) Cylinder {
    float x, y, z, r;       // center + radius
    float vx, vy, vz;       // axis vector
    int materialIndex;       // index into Scene.materialTable
    float invAxisLen;        // precomputed axis to avoid sqrt operations later on
};

// Scene

struct Scene {
    // Materials
    MatteMaterials matte;
    MetalMaterials metal;
    RefractiveMaterials refractive;

    // Primitives
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;

    // Material type enum for lookup
    enum MaterialType { MATTE = 0, METAL = 1, REFRACTIVE = 2 };

    // Material reference table
    struct MaterialRef {
        MaterialType type;
        int localIndex;  // index to the material
    };
    std::vector<MaterialRef> materialTable;

    // Names for debug later on maybe? 
    std::vector<const char*> materialNames;

    // TODO: Reserve capacity for fast parsing
};
