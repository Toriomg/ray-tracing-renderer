#include "../../common/include/camera.hpp"
#include "../../common/include/constants.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/ray.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include "../../common/include/utilities/random.hpp"

#include "image_aos.hpp"
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

std::string const FilepathScene = "/workspace/res/scene_scripts/scene2.txt";
std::string const FilepathOut   = "/workspace/outputImageAOS.ppm";  // ← CAMBIADO: Nombre diferente

int main() {  // NOLINT
  // Placeholders temporales
  // TODO: cambiar por los parsers
  std::shared_ptr<ConfigSettings> config = std::make_shared<ConfigSettings>(ConfigSettings{
    Constants::CameraPosition,        // .camera_pos
    Constants::CameraTarget,          // .camera_target
    Constants::CameraNorth,           // .camera_north
    Constants::FOV,                   // .field_of_view
    Constants::AspectRatio,           // .aspect_ratio
    900,                              // .image_width
    Constants::Gamma,                 // .gamma
    Constants::MaxDepth,              // .max_depth
    Constants::SamplesPerPixel,       // .samples_per_pixel
    Constants::RNGSeedMaterial,       // .material_rng_seed
    Constants::RNGSeedRay,            // .ray_rng_seed
    Constants::ColorBackgroundDark,   // .background_dark_color
    Constants::ColorBackGroundLight,  // .background_light_color
  });
  SceneSettings scene                    = loadSceneFromFile(FilepathScene);

  // Crear randomizadores
  auto rngRay      = RandomGenerator(config->ray_rng_seed);
  auto rngMaterial = RandomGenerator(config->material_rng_seed);

  auto camera = Camera(config);  // Crear la cámara

  // Image width
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  auto pixel_width = camera.ProjWindow.viewportHorizontal * (1.0F / static_cast<float>(imageWidth));
  auto pixel_height = camera.ProjWindow.viewportVertical * (1.0F / static_cast<float>(imageHeight));

  ImageAOS image     = ImageAOS(imageWidth, imageHeight);  // ← CAMBIADO: Usar ImageAOS
  double const scale = 1.0 / static_cast<double>(config->samples_per_pixel);

  /* Esto de aquí es ya la guerra hay q refactorizarlo*/
  for (size_t row = 0; row < (imageHeight); row++) {
    std::cerr << "\rScanlines remaining: " << (imageHeight - 1 - row) << ' ' << std::flush;
    for (size_t col = 0; col < (imageWidth); col++) {
      // Por cada pixel
      Color accumulated_color(0.0F, 0.0F, 0.0F);
      for (int s = 0; s < config->samples_per_pixel; ++s) {
        // Ray position in proj screen
        //  random_double da [0,1), al restarle 0.5 da [-0.5, 0.5)
        float delta_x = rngRay.get_float(-0.5F, 0.5F);
        float delta_y = rngRay.get_float(-0.5F, 0.5F);

        Point3 pixel_sample_point =
            camera.ProjWindow.viewportOrigin +
            pixel_width *
                (static_cast<float>(col) + delta_x) +  // El ancho se multiplica por la columna
            pixel_height * (static_cast<float>(row) + delta_y);

        // Crear el rayo
        Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos);
        // Sacar el color del rayo
        accumulated_color += rayColor(ray, scene, *config, rngMaterial, config->max_depth);
      }

      Color final_pixel_color = accumulated_color * static_cast<float>(scale);

      // Convertir a uint8_t
      uint8_t red   = color_utils::float_to_uint8(final_pixel_color.x);
      uint8_t green = color_utils::float_to_uint8(final_pixel_color.y);
      uint8_t blue  = color_utils::float_to_uint8(final_pixel_color.z);

      // ✅ CAMBIO IMPORTANTE: En AOS usamos set_pixel con (fila, columna)
      image.set_pixel(image.indice(row, col), red, green, blue);
    }
  }

  if (!image.write_to_ppm(FilepathOut)) {
    std::cerr << "Error writing into .ppm file";
    return 1;
  }

  std::cerr << "\nImagen AOS guardada en: " << FilepathOut << "\n";
  return 0;
}
