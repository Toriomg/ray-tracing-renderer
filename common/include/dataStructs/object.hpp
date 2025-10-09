#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>

struct alignas(16) SphereData {
  std::vector<float> x, y, z, r;            // All the numbers that define the sphere
  std::vector<unsigned int> materialIndex;  // Index of its material
};

struct alignas(16) CylinderData {
  std::vector<float> x, y, z, r;  // center + radius
  std::vector<float> vx, vy, vz;  // axis vector
  std::vector<float> invAxisLen;  // precomputed axis to avoid sqrt operations later on
  std::vector<int> materialIndex;

  void addCentre(float cx, float cy, float cz) {
    x.push_back(cx);
    y.push_back(cy);
    z.push_back(cz);
  }

  void addAxis(float avx, float avy, float avz) {
    vx.push_back(avx);
    vy.push_back(avy);
    vz.push_back(avz);
  }
};

#endif
