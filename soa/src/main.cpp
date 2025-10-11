#include "../../common/include/camera.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/renderer.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random.hpp"

#include "image_soa.hpp"
#include <cstddef>
#include <iostream>
#include <string>

std::string const FilepathScene  = "/workspace/res/scene_scripts/scene2.txt";
std::string const FilepathConfig = "/workspace/res/configs/config2.txt";
std::string const FilepathOut    = "/workspace/outputImageSOA.ppm";

int main() {
  // REPLACE the hardcoded config with config parser
  ConfigSettings config = loadConfigFromFile(FilepathConfig);

  // Debug: Print loaded config to verify it works
  std::cout << "=== Loaded Configuration ===" << '\n';
  std::cout << "Image width: " << config.image_width << '\n';
  std::cout << "Samples per pixel: " << config.samples_per_pixel << '\n';
  std::cout << "Max depth: " << config.max_depth << '\n';
  std::cout << "Camera position: " << config.camera_pos.x << ", " << config.camera_pos.y << ", "
            << config.camera_pos.z << '\n';
  std::cout << "============================" << '\n';

  // Create shared_ptr for Camera (if Camera requires shared_ptr)
  std::shared_ptr<ConfigSettings> config_ptr = std::make_shared<ConfigSettings>(config);

  SceneSettings scene = loadSceneFromFile(FilepathScene);

  // Crear randomizadores - use config (object) instead of config->
  auto rngRay      = RandomGenerator(config.ray_rng_seed);
  auto rngMaterial = RandomGenerator(config.material_rng_seed);

  auto camera = Camera(config_ptr);  // Crear la cámara

  // Image width
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  auto pixel_width = camera.ProjWindow.viewportHorizontal * (1.0F / static_cast<float>(imageWidth));
  auto pixel_height = camera.ProjWindow.viewportVertical * (1.0F / static_cast<float>(imageHeight));

  ImageSOA image     = ImageSOA(imageWidth, imageHeight);
  double const scale = 1.0 / static_cast<double>(config.samples_per_pixel);  // Use config.

  /* Esto de aquí es ya la guerra hay q refactorizarlo*/
  for (size_t row = 0; row < (imageHeight); row++) {
    std::cerr << "\rScanlines remaining: " << (imageHeight - 1 - row) << ' ' << std::flush;
    for (size_t col = 0; col < (imageWidth); col++) {
      // Por cada pixel
      Color accumulated_color(0.0F, 0.0F, 0.0F);
      for (int s = 0; s < config.samples_per_pixel; ++s) {  // Use config.
        // Posición del rayo en la pantalla
        // random_double da [0,1), al restarle 0.5 da [-0.5, 0.5)
        float delta_x = rngRay.get_float(-0.5F, 0.5F);
        float delta_y = rngRay.get_float(-0.5F, 0.5F);

        Point3 pixel_sample_point =
            camera.ProjWindow.viewportOrigin +
            pixel_width *
                (static_cast<float>(col) + delta_x) +  // El ancho se multiplica por la columna
            pixel_height * (static_cast<float>(row) + delta_y);

        // Crear el rayo
        Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos,
                config.max_depth);  // Use config.
        // Sacar el color del rayo
        accumulated_color += Renderer::rayColor(ray, scene, config, rngMaterial);  // Use config
      }

      Color final_pixel_color = accumulated_color * static_cast<float>(scale);
      // guardamos la imagen
      size_t indice = image.indice(row, col);
      image.set_pixel(indice, final_pixel_color);
    }
  }
  std::cerr << "\n";
  if (!image.write_to_ppm(FilepathOut)) {
    std::cerr << "Error writing into .ppm file /n";
  }
}
