#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "utilities/vec3.hpp"
#include <limits>
#include <numbers>
#include <utility>

namespace Constants {

  inline constexpr double PI       = std::numbers::pi_v<double>;
  inline constexpr double Infinity = std::numeric_limits<double>::infinity();

  inline constexpr std::pair<unsigned int, unsigned int> AspectRatio = {16, 9};
  inline constexpr int ImageWidth                                    = 1'920;
  inline constexpr Point3 CameraPosition(0.0F, 0.0F, -10.0F);
  inline constexpr Point3 CameraTarget(0.0F, 0.0F, 0.0F);
  inline constexpr Vec3 CameraNorth(0.0F, 1.0F, 0.0F);
  // Field of view
  inline constexpr double FOV          = 90.0;
  inline constexpr double Gamma        = 2.2;
  inline constexpr int SamplesPerPixel = 20;
  inline constexpr int MaxDepth        = 5;
  // TODO: select a correct integer type
  inline constexpr unsigned int RNGSeedMaterial = 13;
  inline constexpr unsigned int RNGSeedRay      = 19;
  inline constexpr Color ColorBackGroundLight(1.0F, 1.0F, 1.0F);
  inline constexpr Color ColorBackgroundDark(0.25F, 0.5F, 1.0F);

  inline constexpr int MAX_PRIMS_IN_NODE = 4;

}  // namespace Constants

#endif
