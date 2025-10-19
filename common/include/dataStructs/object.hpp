#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <cmath>
#include <vector>

struct alignas(16) SphereData {
  std::vector<double> x, y, z, r;           // All the numbers that define the sphere
  std::vector<unsigned int> materialIndex;  // Index of its material
};

struct alignas(16) CylinderData {
  std::vector<double> x, y, z, r;
  std::vector<double> vx, vy, vz;
  std::vector<double> invAxisLen;
  std::vector<int> materialIndex;

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

  // Optional convenience method
  // void addCylinder(double cx, double cy, double cz,
  //                 double avx, double avy, double avz,
  //                 double radius, int materialIdx) {
  //   addCentre(cx, cy, cz);
  //   addAxis(avx, avy, avz);
  //   r.push_back(radius);
  //   materialIndex.push_back(materialIdx);
  // }
};

#endif
