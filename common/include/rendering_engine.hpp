#ifndef RENDERING_ENGINE_HPP
#define RENDERING_ENGINE_HPP

#include "../../common/include/camera.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/renderer.hpp"
#include "../../common/include/utilities/random.hpp"
#include "../../common/include/utilities/random_par.hpp"
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

// RenderContext struct
struct RenderContext {
  SceneSettings * scene;
  ConfigSettings const * config;
  ParallelRNGManager * rng_manager;

  RenderContext(SceneSettings * scn, ConfigSettings const * cfg, ParallelRNGManager * rng_mgr)
      : scene(scn), config(cfg), rng_manager(rng_mgr) { }

  // Métodos NUEVOS de conveniencia:
  [[nodiscard]] RandomGenerator & get_ray_rng() const { return rng_manager->get_ray_rng(); }

  [[nodiscard]] RandomGenerator & get_material_rng() const {
    return rng_manager->get_material_rng();
  }
};

// EN rendering_engine.hpp

template <typename ImageType>
void renderImage(ImageType & image, Camera & camera, RenderContext & ctx) {
  auto imageWidth    = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight   = static_cast<size_t>(camera.ProjWindow.imageHeight);
  auto pixel_delta_u = camera.ProjWindow.viewportHorizontal / static_cast<double>(imageWidth);
  auto pixel_delta_v = camera.ProjWindow.viewportVertical / static_cast<double>(imageHeight);

  // Origen de la ventana de proyección (esquina superior izquierda del píxel 0,0)
  auto pixel00_loc = camera.ProjWindow.viewportOrigin + 0.5 * (pixel_delta_u + pixel_delta_v);

  double const scale = 1.0 / static_cast<double>(ctx.config->samples_per_pixel);

  tbb::parallel_for(
      tbb::blocked_range2d<size_t>(0, imageHeight, 0, imageWidth),
      [&](tbb::blocked_range2d<size_t> const & r) {
        // Cada hilo obtiene sus propios generadores locales
        auto & ray_rng      = ctx.get_ray_rng();
        auto & material_rng = ctx.get_material_rng();

        for (size_t row = r.rows().begin(); row != r.rows().end(); ++row) {
          for (size_t col = r.cols().begin(); col != r.cols().end(); ++col) {
            Color accumulated_color(0.0, 0.0, 0.0);

            Point3 const pixel_corner = pixel00_loc +  // LOC pixel actual
                                        (static_cast<double>(col) * pixel_delta_u) +
                                        (static_cast<double>(row) * pixel_delta_v);
            for (int s = 0; s < ctx.config->samples_per_pixel; ++s) {
              // Genera un punto aleatorio DENTRO del cuadrado del píxel
              double const px = ray_rng.get_double() - 0.5;
              double const py = ray_rng.get_double() - 0.5;

              Point3 const pixel_sample_point =
                  pixel_corner + (px * pixel_delta_u) + (py * pixel_delta_v);
              Ray const ray(camera.cameraPos, pixel_sample_point - camera.cameraPos,
                            ctx.config->max_depth);

              // Pasar material_rng por referencia
              accumulated_color += Renderer::rayColor(ray, *ctx.scene, *ctx.config, material_rng);
            }
            Color const final_pixel_color = accumulated_color * static_cast<double>(scale);

            size_t const index = image.indice(row, col);
            image.set_pixel(index, final_pixel_color, ctx.config->gamma);
          }
        }
      });
}

#endif
