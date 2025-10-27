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

  for (size_t row = 0; row < imageHeight; row++) {
    std::cerr << "\rScanlines remaining: " << imageHeight - 1 - row
              << ". Porcentage: " << 100 * (imageHeight - 1 - row) / imageHeight << "% "
              << std::flush;
    for (size_t col = 0; col < imageWidth; col++) {
      Color accumulated_color(0.0, 0.0, 0.0);

      Point3 pixel_corner = pixel00_loc +  // LOC pixel actual
                            (static_cast<double>(col) * pixel_delta_u) +
                            (static_cast<double>(row) * pixel_delta_v);
      for (int s = 0; s < ctx.config->samples_per_pixel; ++s) {
        // Genera un punto aleatorio DENTRO del cuadrado del píxel
        double px = ctx.rngRay->get_double() - 0.5;  // [-0.5, 0.5)
        double py = ctx.rngRay->get_double() - 0.5;  // [-0.5, 0.5)

        Point3 pixel_sample_point = pixel_corner + (px * pixel_delta_u) + (py * pixel_delta_v);
        Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos, ctx.config->max_depth);
        accumulated_color += Renderer::rayColor(ray, *ctx.scene, *ctx.config, *ctx.rngMaterial);
      }
      Color final_pixel_color = accumulated_color * static_cast<double>(scale);

      size_t index = image.indice(row, col);
      image.set_pixel(index, final_pixel_color, ctx.config->gamma);
    }
  }
  std::cerr << "\n";
}

#endif
