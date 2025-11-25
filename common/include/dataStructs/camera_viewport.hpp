#ifndef CAMERA_VIEWPORT_HPP
#define CAMERA_VIEWPORT_HPP

#include "../utilities/vec3.hpp"
#include <cstdint>

struct CameraData {
  Point3 position;
  Point3 target;
  Vec3 north;
  double FOV_radians{};
  double aspect_width{};
  double aspect_height{};
  uint32_t window_width{};
  uint32_t window_height{};
};

struct ViewportData {
  Vec3 right;
  Vec3 up;
  Vec3 horizontal;
  Vec3 vertical;
  Point3 origin;
  Point3 position;
};

struct WindowData {
  uint32_t width;
  uint32_t height;
};

ViewportData compute_viewport(CameraData const & cam);

#endif
