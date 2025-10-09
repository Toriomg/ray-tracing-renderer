#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <vector>

struct MatteMaterials {
  std::vector<float> r, g, b;
};

struct MetalMaterials {
  std::vector<float> r, g, b, diffusion;
};

struct RefractiveMaterials {
  std::vector<float> ior;
};

enum MaterialType { MATTE = 0, METAL = 1, REFRACTIVE = 2 };

// Material reference table
struct MaterialID {
  // Selecciona el SOA
  MaterialType type;
  // Accede al índice del SOA
  unsigned int localIndex;
};

#endif
