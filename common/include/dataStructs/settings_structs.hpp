#ifndef SETTINGS_STRUCTS_HPP
#define SETTINGS_STRUCTS_HPP

#include "../utilities/vec3.hpp"
#include "bvh.hpp"
#include "constants.hpp"
#include "material.hpp"
#include "object.hpp"
#include <vector>

struct ConfigSettings {
  Point3 camera_pos;
  Point3 camera_target;
  Vec3 camera_north;
  double field_of_view = Constants::FOV;
  std::pair<unsigned int, unsigned int> aspect_ratio;
  int image_width                 = Constants::ImageWidth;
  double gamma                    = Constants::Gamma;
  int max_depth                   = Constants::MaxDepth;
  int samples_per_pixel           = Constants::SamplesPerPixel;
  unsigned long material_rng_seed = Constants::RNGSeedMaterial;
  unsigned long ray_rng_seed      = Constants::RNGSeedRay;
  Color background_dark_color;
  Color background_light_color;
};

struct SceneSettings {
  SphereData spheres;
  CylinderData cylinders;
  std::vector<MaterialID> materialTable;
  std::vector<std::string> materialNames;  // Just for debugging
  std::shared_ptr<BVH> bvh;

  // tiene los SOA de los materiales
  MatteMaterials matte;
  MetalMaterials metal;
  RefractiveMaterials refractive;
};

#endif
