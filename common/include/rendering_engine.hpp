#ifndef RENDERING_ENGINE_HPP
#define RENDERING_ENGINE_HPP

#include "../../common/include/camera.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/renderer.hpp"
#include "../../common/include/utilities/random.hpp"
#include "../../common/include/utilities/random_par.hpp"
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>

// RenderContext struct
struct RenderContext {
  SceneSettings * scene;
  ConfigSettings const * config;
  ParallelRNGManager * rng_manager;
  ParallelSettings par_settings;

  RenderContext(SceneSettings * scn, ConfigSettings const * cfg, ParallelRNGManager * rng_mgr,
                ParallelSettings const * par_settings = nullptr)
      : scene(scn), config(cfg), rng_manager(rng_mgr),
        par_settings(par_settings != nullptr ? *par_settings : ParallelSettings{}) { }

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
  auto pixel00_loc   = camera.ProjWindow.viewportOrigin + 0.5 * (pixel_delta_u + pixel_delta_v);

  double const scale = 1.0 / static_cast<double>(ctx.config->samples_per_pixel);

  // Create 2D range with custom grain size if specified
  tbb::blocked_range2d<size_t> range =
      (ctx.par_settings.grainSize > 0)
          ? tbb::blocked_range2d<size_t>(0, imageHeight, ctx.par_settings.grainSize, 0, imageWidth,
                                         ctx.par_settings.grainSize)
          : tbb::blocked_range2d<size_t>(0, imageHeight, 0, imageWidth);

  // Lambda for pixel rendering
  auto render_lambda = [&](tbb::blocked_range2d<size_t> const & r) {
    // Cada hilo obtiene sus propios generadores locales
    auto & ray_rng      = ctx.get_ray_rng();
    auto & material_rng = ctx.get_material_rng();

    for (size_t row = r.rows().begin(); row != r.rows().end(); ++row) {
      for (size_t col = r.cols().begin(); col != r.cols().end(); ++col) {
        Color accumulated_color(0.0, 0.0, 0.0);
        Point3 const pixel_corner = pixel00_loc +  // LOC pixel actual
                                    (static_cast<double>(col) * pixel_delta_u) +
                                    (static_cast<double>(row) * pixel_delta_v);
        for (int s = 0; s < ctx.config->samples_per_pixel; ++s) {  // punto aleatorio dentro
          double const px = ray_rng.get_double() - 0.5;
          double const py = ray_rng.get_double() - 0.5;

          Point3 const pixel_sample_point =
              pixel_corner + (px * pixel_delta_u) + (py * pixel_delta_v);
          Ray const ray(camera.cameraPos, pixel_sample_point - camera.cameraPos,
                        ctx.config->max_depth);
          accumulated_color += Renderer::rayColor(ray, *ctx.scene, *ctx.config, material_rng);
        }
        Color const final_pixel_color = accumulated_color * static_cast<double>(scale);

        size_t const index = image.indice(row, col);
        image.set_pixel(index, final_pixel_color, ctx.config->gamma);
      }
    }
  };

  // Select partitioner based on settings
  switch (ctx.par_settings.type) {
    case PartitionerType::Simple:
      tbb::parallel_for(range, render_lambda, tbb::simple_partitioner());
      break;
    case PartitionerType::Static:
      tbb::parallel_for(range, render_lambda, tbb::static_partitioner());
      break;
    case PartitionerType::Affinity:
    {
      static tbb::affinity_partitioner affinity_part;
      tbb::parallel_for(range, render_lambda, affinity_part);
      break;
    }
    case PartitionerType::Auto:
    default:                    tbb::parallel_for(range, render_lambda, tbb::auto_partitioner()); break;
  }
}

#endif
