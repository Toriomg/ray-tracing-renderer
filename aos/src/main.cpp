#include "../../common/include/config_parser.hpp"
#include "./../include/rendering_engine.hpp"
#include "./../include/scene_parser.hpp"
#include "./../include/config_parser.hpp"
#include "image_aos.hpp"
#include <iostream>
#include <string>

// File paths
std::string const FilepathScene  = "/workspace/res/scene_scripts/scene2.txt";
std::string const FilepathConfig = "/workspace/res/configs/config2.txt";
std::string const FilepathOutAOS = "/workspace/outputImageAOS.ppm";

int main() {
  // Load configuration and scene
  ConfigSettings config = loadConfigFromFile(FilepathConfig);
  SceneSettings scene   = loadSceneFromFile(FilepathScene);

  // Create random generators
  auto rngRay      = RandomGenerator(config.ray_rng_seed);
  auto rngMaterial = RandomGenerator(config.material_rng_seed);

  // Create camera
  auto camera      = Camera(config);
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  // Create render context
  RenderContext ctx(&scene, &config, &rngRay, &rngMaterial);

  // Render with ImageAOS
  {
    std::cout << "Rendering with ImageAOS..." << '\n';
    ImageAOS imageAos(imageWidth, imageHeight);
    renderImage(imageAos, camera, ctx);
    if (!imageAos.write_to_ppm(FilepathOutAOS)) {
      std::cerr << "Error writing ImageAOS to .ppm file\n";
    }
  }

  return 0;
}
