#include "constants.hpp"
#include "dataStructs/camera_viewport.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <vector>

int main() {
  // Camera setup
  CameraData cam{
    Constants::CameraPosition,                  // Camera position
    Constants::CameraTarget,                    // Camera target
    Constants::CameraNorth,                     // Up direction
    Constants::FOV * (Constants::PI / 180.0F),  // FOV in radians
    (double) Constants::AspectRatio.first,       // Aspect width
    (double) Constants::AspectRatio.second,      // Aspect height
    1'920,                                      // Window width
    700                                         // Window height
  };

  // Compute viewport from camera
  ViewportData vp = compute_viewport(cam);

  // Pixel loop and sample point calculation
  for (unsigned int pixelCountX = 0; pixelCountX < cam.window_width; pixelCountX++) {
    for (unsigned int pixelCountY = 0; pixelCountY < cam.window_height; pixelCountY++) {
      // Pixel size in world space
      Vec3 pixel_width  = vp.horizontal / (double) cam.window_width;
      Vec3 pixel_height = vp.vertical / (double) cam.window_height;

      // Generate sample points (currently empty or 1 sample)
      std::vector<Point3> sample_points(1);
      unsigned delta_x_random = 0;  // δx
      unsigned delta_y_random = 0;  // δy

      for (Point3 & pixel_sample_point : sample_points) {
        pixel_sample_point = vp.origin +
                             pixel_width * (double) (pixelCountX + delta_x_random) +
                             pixel_height * (double) (pixelCountY + delta_y_random);
      }

      // TODO: do something with sample_points
    }
  }

  return 0;
}
