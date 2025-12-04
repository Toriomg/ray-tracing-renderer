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

// Pixel Renderer Functor
template <typename ImageType> class PixelRenderer {
public:
  PixelRenderer(ImageType & img, Camera & cam, RenderContext & context)
      : image_(img), camera_(cam), ctx_(context),
        image_width_(static_cast<size_t>(cam.ProjWindow.imageWidth)),
        image_height_(static_cast<size_t>(cam.ProjWindow.imageHeight)),
        pixel_delta_u_(cam.ProjWindow.viewportHorizontal / static_cast<double>(image_width_)),
        pixel_delta_v_(cam.ProjWindow.viewportVertical / static_cast<double>(image_height_)),
        pixel00_loc_(cam.ProjWindow.viewportOrigin + 0.5 * (pixel_delta_u_ + pixel_delta_v_)),
        scale_(1.0 / static_cast<double>(context.config->samples_per_pixel)) { }

  void operator()(tbb::blocked_range2d<size_t> const & r) const {
    // Each thread gets its own local RNG generators
    auto & ray_rng      = ctx_.get_ray_rng();
    auto & material_rng = ctx_.get_material_rng();

    for (size_t row = r.rows().begin(); row != r.rows().end(); ++row) {
      for (size_t col = r.cols().begin(); col != r.cols().end(); ++col) {
        Color accumulated_color(0.0, 0.0, 0.0);
        Point3 const pixel_corner = pixel00_loc_ +
                                    (static_cast<double>(col) * pixel_delta_u_) +
                                    (static_cast<double>(row) * pixel_delta_v_);

        for (int s = 0; s < ctx_.config->samples_per_pixel; ++s) {
          double const px = ray_rng.get_double() - 0.5;
          double const py = ray_rng.get_double() - 0.5;

          Point3 const pixel_sample_point =
              pixel_corner + (px * pixel_delta_u_) + (py * pixel_delta_v_);
          Ray const ray(camera_.cameraPos, pixel_sample_point - camera_.cameraPos,
                        ctx_.config->max_depth);
          accumulated_color += Renderer::rayColor(ray, *ctx_.scene, *ctx_.config, material_rng);
        }

        Color const final_pixel_color = accumulated_color * scale_;
        size_t const index            = image_.indice(row, col);
        image_.set_pixel(index, final_pixel_color, ctx_.config->gamma);
      }
    }
  }

private:
  ImageType & image_;
  Camera & camera_;
  RenderContext & ctx_;
  size_t const image_width_;
  size_t const image_height_;
  Vec3 const pixel_delta_u_;
  Vec3 const pixel_delta_v_;
  Point3 const pixel00_loc_;
  double const scale_;
};

// Main rendering function
template <typename ImageType>
void renderImage(ImageType & image, Camera & camera, RenderContext & ctx) {
  auto const imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);
  auto const imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);

  // Create 2D range with custom grain size if specified
  tbb::blocked_range2d<size_t> range =
      (ctx.par_settings.grainSize > 0)
          ? tbb::blocked_range2d<size_t>(0, imageHeight, ctx.par_settings.grainSize, 0, imageWidth,
                                         ctx.par_settings.grainSize)
          : tbb::blocked_range2d<size_t>(0, imageHeight, 0, imageWidth);

  // Create pixel renderer functor
  PixelRenderer<ImageType> renderer(image, camera, ctx);

  // Select partitioner based on settings
  switch (ctx.par_settings.type) {
    case PartitionerType::Simple:
      tbb::parallel_for(range, renderer, tbb::simple_partitioner());
      break;
    case PartitionerType::Static:
      tbb::parallel_for(range, renderer, tbb::static_partitioner());
      break;
    case PartitionerType::Affinity:
    {
      static tbb::affinity_partitioner affinity_part;
      tbb::parallel_for(range, renderer, affinity_part);
      break;
    }
    case PartitionerType::Auto:
    default:                    tbb::parallel_for(range, renderer, tbb::auto_partitioner()); break;
  }
}

#endif
