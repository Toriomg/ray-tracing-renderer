#ifndef RENDERING_ENGINE_HPP
#define RENDERING_ENGINE_HPP

#include "../../aos/include/image_aos.hpp"
#include "../../common/include/camera.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/renderer.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random.hpp"
#include "../../soa/include/image_soa.hpp"
#include <iostream>
#include <memory>

// RenderContext struct
struct RenderContext {
  SceneSettings * scene;
  ConfigSettings * config;
  RandomGenerator * rngRay;
  RandomGenerator * rngMaterial;

  RenderContext(SceneSettings * scn, ConfigSettings * cfg, RandomGenerator * rngR,
                RandomGenerator * rngM)
      : scene(scn), config(cfg), rngRay(rngR), rngMaterial(rngM) { }
};

// Template function implementation IN THE HEADER
template <typename ImageType>
void renderImage(ImageType & image, Camera & camera, RenderContext & ctx) {
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  auto pixel_width = camera.ProjWindow.viewportHorizontal * (1.0F / static_cast<double>(imageWidth));
  auto pixel_height = camera.ProjWindow.viewportVertical * (1.0F / static_cast<double>(imageHeight));
  double const scale = 1.0 / static_cast<double>(ctx.config->samples_per_pixel);

  for (size_t row = 0; row < imageHeight; row++) {
    std::cerr << "\rScanlines remaining: " << (imageHeight - 1 - row) << ' ' << std::flush;
    for (size_t col = 0; col < imageWidth; col++) {
      Color accumulated_color(0.0F, 0.0F, 0.0F);
      for (int s = 0; s < ctx.config->samples_per_pixel; ++s) {
        double delta_x = ctx.rngRay->get_double(-0.5F, 0.5F);
        double delta_y = ctx.rngRay->get_double(-0.5F, 0.5F);

        Point3 pixel_sample_point = camera.ProjWindow.viewportOrigin +
                                    pixel_width * (static_cast<double>(col) + delta_x) +
                                    pixel_height * (static_cast<double>(row) + delta_y);

        Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos, ctx.config->max_depth);
        accumulated_color += Renderer::rayColor(ray, *ctx.scene, *ctx.config, *ctx.rngMaterial);
      }

      Color final_pixel_color = accumulated_color * static_cast<double>(scale);

      size_t index = image.indice(row, col);
      image.set_pixel(index, final_pixel_color);
    }
  }
}

#endif
