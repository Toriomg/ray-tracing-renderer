#include "constants.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <vector>

int main() {
  Point3 camera_position = Constants::CameraPosition;
  Point3 camera_target   = Constants::CameraTarget;
  float FOV_radians      = Constants::FOV * (Constants::PI / 180.0F);
  // aspect ratio es un std::pair: 1º anchura, 2º altura
  std::pair aspect_ratio           = Constants::AspectRatio;
  Vec3 camera_north                = Constants::CameraNorth;
  unsigned int const window_height = 1'920;
  unsigned int const window_width  = 700;

  Vec3 focal_vector = camera_position - camera_target;

  float focal_distance = focal_vector.length();

  // Altura de la ventana de proyección
  float proj_window_height = 2 * std::tan(FOV_radians / 2.0F) * focal_distance;
  // Anchura de la ventana de proyección
  float proj_window_width = proj_window_height * aspect_ratio.first / aspect_ratio.second;

  // Vectores directores de la ventana
  Vec3 focal_vector_norm = focal_vector.normalize();
  Vec3 viewport_right    = cross(camera_north, focal_vector_norm).normalize();
  Vec3 viewport_up       = cross(focal_vector_norm, viewport_right);
  // Vectores de la ventana de proyección
  Vec3 viewport_horizontal = proj_window_width * viewport_right;
  Vec3 viewport_vertical   = proj_window_height * -viewport_up;

  // Origen ventana de proyección.
  Vec3 pixel_width         = viewport_horizontal / window_width;
  Vec3 pixel_height        = viewport_vertical / window_height;
  Point3 viewport_origin   = camera_target - 0.5F * (viewport_horizontal + viewport_vertical);
  Point3 viewport_position = viewport_origin + 0.5F * (pixel_width + pixel_height);

  /* Trazado de rayos */
  for (unsigned int pixelCountX = 0; pixelCountX < window_width; pixelCountX++) {
    for (unsigned int pixelCountY = 0; pixelCountY < window_height; pixelCountY++) {
      std::vector<Point3> sample_points;
      // should be random
      auto delta_x_random = 0;  // δx
      auto delta_y_random = 0;  // δy
      for (Point3 & pixel_sample_point : sample_points) {
        pixel_sample_point = viewport_origin +
                             pixel_width * (pixelCountX + delta_x_random) +
                             pixel_height * (pixelCountY + delta_y_random);
      }
    }
  }
}
