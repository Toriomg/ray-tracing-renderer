#ifndef RENDERING_ENGINE_HPP
#define RENDERING_ENGINE_HPP

#include "camera.hpp"
#include "dataStructs/settings_structs.hpp"
#include "renderer.hpp"
#include "utilities/random.hpp"
#include "utilities/random_par.hpp"

// Rama analysis/image: rendering_engine es SECUENCIAL (sin TBB)
// Solo ImagePar usa paralelización en esta rama

// RenderContext struct
struct RenderContext {
  SceneSettings * scene;
  ConfigSettings const * config;
  ParallelRNGManager * rng_manager;
  ParallelSettings par_settings;  // Ignored in this branch (rendering is sequential)

  RenderContext(SceneSettings * scn, ConfigSettings const * cfg, ParallelRNGManager * rng_mgr,
                ParallelSettings const * par_settings = nullptr)
      : scene(scn), config(cfg), rng_manager(rng_mgr),
        par_settings(par_settings != nullptr ? *par_settings : ParallelSettings{}) { }

  [[nodiscard]] RandomGenerator & get_ray_rng() const { return rng_manager->get_ray_rng(); }

  [[nodiscard]] RandomGenerator & get_material_rng() const {
    return rng_manager->get_material_rng();
  }
};

// Rama analysis/image: Rendering SECUENCIAL con bucles for simples
template <typename ImageType>
void renderImage(ImageType & image, Camera & camera, RenderContext & ctx) {
  auto const imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto const imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  Vec3 const pixel_delta_u = camera.ProjWindow.viewportHorizontal / static_cast<double>(imageWidth);
  Vec3 const pixel_delta_v = camera.ProjWindow.viewportVertical / static_cast<double>(imageHeight);
  Vec3 const pixel00_loc   = camera.ProjWindow.viewportOrigin + 0.5 * (pixel_delta_u + pixel_delta_v);
  double const scale       = 1.0 / static_cast<double>(ctx.config->samples_per_pixel);

  auto & ray_rng      = ctx.get_ray_rng();
  auto & material_rng = ctx.get_material_rng();

  // BUCLE SECUENCIAL: Procesamos cada píxel uno por uno
  for (size_t row = 0; row < imageHeight; ++row) {
    for (size_t col = 0; col < imageWidth; ++col) {
      Color accumulated_color(0.0, 0.0, 0.0);
      Point3 const pixel_corner = pixel00_loc + (static_cast<double>(col) * pixel_delta_u) +
                                  (static_cast<double>(row) * pixel_delta_v);

      for (int s = 0; s < ctx.config->samples_per_pixel; ++s) {
        double const px                   = ray_rng.get_double() - 0.5;
        double const py                   = ray_rng.get_double() - 0.5;
        Point3 const pixel_sample_point   = pixel_corner + (px * pixel_delta_u) + (py * pixel_delta_v);
        Ray const ray(camera.cameraPos, pixel_sample_point - camera.cameraPos, ctx.config->max_depth);

        accumulated_color += Renderer::rayColor(ray, *ctx.scene, *ctx.config, material_rng);
      }

      size_t const index = image.indice(row, col);
      image.set_pixel(index, accumulated_color * scale);
    }
  }
}

#endif
