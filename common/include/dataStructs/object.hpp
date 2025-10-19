#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "dataStructs/aabb.hpp"
#include <vector>

struct alignas(16) SphereData {
  std::vector<double> x, y, z, r;           // All the numbers that define the sphere
  std::vector<unsigned int> materialIndex;  // Index of its material
  std::vector<AABB> aabbs;                  // Caja AABB de la esfera para colisiones
};

struct alignas(16) CylinderData {
  std::vector<double> x, y, z, r;  // center + radius
  std::vector<double> vx, vy, vz;  // axis vector
  std::vector<double> invAxisLen;  // precomputed axis to avoid sqrt operations later on
  std::vector<int> materialIndex;
  std::vector<AABB> aabbs;  // caja AABB del cilindro para colisiones

  void addCentre(double cx, double cy, double cz) {
    x.push_back(cx);
    y.push_back(cy);
    z.push_back(cz);
  }

  void addAxis(double avx, double avy, double avz) {
    vx.push_back(avx);
    vy.push_back(avy);
    vz.push_back(avz);
  }

  void addAABB(AABB const & aabb) {  // ← AÑADE ESTE MÉTODO
    aabbs.push_back(aabb);
  }
};

#endif
