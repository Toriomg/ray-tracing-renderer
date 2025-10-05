#pragma once

#include <vector>

struct alignas(16) SphereData {
    std::vector<float> x, y, z, r;       // All the numbers that define the sphere
    std::vector<unsigned int> materialIndex;       // Index of its material
};

struct alignas(16) CylinderData {
    std::vector<float> x, y, z, r;       // center + radius
    std::vector<float> vx, vy, vz;       // axis vector
    std::vector<unsigned int> invAxisLen;        // precomputed axis to avoid sqrt operations later on
    std::vector<int> materialIndex;
};