#pragma once

#include <string>
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
  float x{};
  float y{};
  float z{};
  float r{};
  int materialIndex{};
};
struct alignas(16) Cylinder {
  float x{};
  float y{};
  float z{};
  float r{};
  float vx{};
  float vy{};
  float vz{};
  int materialIndex{};
  float invAxisLen{};
};

// Main Scene struct
struct Scene {
  MatteMaterials matte;
  MetalMaterials metal;
  RefractiveMaterials refractive;
  std::vector<Sphere> spheres;
  std::vector<Cylinder> cylinders;
  enum class MaterialType { MATTE = 0, METAL = 1, REFRACTIVE = 2 };
  struct MaterialRef {
    MaterialType type{};
    int localIndex{};
  };
  std::vector<MaterialRef> materialTable;
  std::vector<std::string> materialNames;
};

[[nodiscard]] Scene loadSceneFromFile(const std::string &filename);