#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "dataStructs/aabb.hpp"
#include <cmath>
#include <vector>

struct alignas(16) SphereData {
  std::vector<double> x, y, z, r;           // All the numbers that define the sphere
  std::vector<unsigned int> materialIndex;  // Index of its material
  std::vector<AABB> aabbs;                  // Caja AABB de la esfera para colisiones
};

struct alignas(16) CylinderData {
  std::vector<double> x, y, z, r;
  std::vector<double> vx, vy, vz;
  std::vector<double> invAxisLen;
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

    // Compute and store inverse axis length
    double const length = std::sqrt(avx * avx + avy * avy + avz * avz);
    if (length > 0.0) {
      invAxisLen.push_back(1.0 / length);
    } else {
      invAxisLen.push_back(1.0);  // Fallback for zero-length vector
    }
  }

  void addAABB(AABB const & aabb) { aabbs.push_back(aabb); }
};

#endif
