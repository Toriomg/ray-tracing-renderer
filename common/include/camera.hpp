#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "utilities/vec3.hpp"

struct ConfigSettings;

struct ProjectionWindow {
  // Initialize members to default values to prevent garbage data.
  double projWindowHeight = 0.0F;
  double projWindowWidth  = 0.0F;
  int imageHeight         = 0;
  int imageWidth          = 0;
  Vec3 viewportHorizontal;
  Vec3 viewportVertical;
  Point3 viewportOrigin;
};

class Camera {
public:
  Vec3 cameraPos;
  Point3 cameraTarget;
  Vec3 cameraNorth;
  double FOV = 0.0F;
  ProjectionWindow ProjWindow;
  Vec3 cameraRight;
  Vec3 cameraUp;
  Vec3 focalVector;

  Camera(ConfigSettings const & config);
};

#endif
