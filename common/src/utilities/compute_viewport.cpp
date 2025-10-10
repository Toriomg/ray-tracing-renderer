#include "dataStructs/camera_viewport.hpp"

ViewportData compute_viewport(CameraData const & cam) {
  ViewportData vp;

  // Vector from camera target to camera position
  Vec3 focal           = cam.position - cam.target;
  float focal_distance = focal.length();

  // Projection window based on FOV and distance
  float proj_h = 2 * std::tan(cam.FOV_radians / 2.0F) * focal_distance;
  float proj_w = proj_h * (cam.aspect_width / cam.aspect_height);

  // Normalize the focal vector to get the forward direction
  Vec3 focal_norm = focal.normalize();

  // Right vector of the viewport: perpendicular to camera north and forward
  vp.right = cross(cam.north, focal_norm).normalize();
  vp.up    = cross(focal_norm, vp.right);

  // Scale viewport axes by the projection window size
  vp.horizontal = proj_w * vp.right;  // horizontal extent in world space
  vp.vertical   = proj_h * -vp.up;    // vertical extent

  // Compute the origin (bottom-left corner)
  vp.origin = cam.target - 0.5F * (vp.horizontal + vp.vertical);

  // Compute the position of the center of the first pixel
  vp.position = vp.origin + 0.5F * (vp.horizontal / (float) cam.window_width +
                                    vp.vertical / (float) cam.window_height);

  return vp;
}
