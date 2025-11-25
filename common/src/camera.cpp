#include "camera.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <utility>

Camera::Camera(ConfigSettings const & config)
    : cameraPos{config.camera_pos}, cameraTarget{config.camera_target},
      cameraNorth{config.camera_north}, FOV{config.field_of_view} {
  focalVector = cameraPos - cameraTarget;

  double const FOV_radians    = FOV * (Constants::PI / 180.0F);
  ProjWindow.projWindowHeight = 2 * std::tan(FOV_radians / 2.0F) * focalVector.length();

  std::pair const AspRt      = config.aspect_ratio;
  ProjWindow.projWindowWidth = ProjWindow.projWindowHeight * (static_cast<double>(AspRt.first) /
                                                              static_cast<double>(AspRt.second));
  // Vectores directores de la ventana
  Vec3 const focalVectorNorm = focalVector.normalize();
  cameraRight                = cross(cameraNorth, focalVectorNorm).normalize();
  cameraUp                   = cross(focalVectorNorm, cameraRight);

  // Vectores de la ventana de proyección
  ProjWindow.viewportHorizontal = ProjWindow.projWindowWidth * cameraRight;
  ProjWindow.viewportVertical   = ProjWindow.projWindowHeight * -cameraUp;

  ProjWindow.imageWidth = config.image_width;
  ProjWindow.imageHeight =
      static_cast<int>(static_cast<double>(ProjWindow.imageWidth) *
                       (static_cast<double>(AspRt.second) / static_cast<double>(AspRt.first)));

  ProjWindow.viewportOrigin =
      cameraTarget - 0.5F * (ProjWindow.viewportHorizontal + ProjWindow.viewportVertical);
}
