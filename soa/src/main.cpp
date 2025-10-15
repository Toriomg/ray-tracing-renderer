#include "../../common/include/config_parser.hpp"
#include "./../include/rendering_engine.hpp"
#include <iostream>
#include <string>

// File paths
std::string const FilepathScene  = "./res/scene_scripts/scene2.txt";
std::string const FilepathConfig = "./res/config_scripts/config2.txt";
std::string const FilepathOut    = "./outputImageSOA.ppm";

int main() {
  // Load configuration and scene
  ConfigSettings config = loadConfigFromFile(FilepathConfig);
  SceneSettings scene   = loadSceneFromFile(FilepathScene);

  // Create shared_ptr for Camera
  std::shared_ptr<ConfigSettings> config_ptr = std::make_shared<ConfigSettings>(config);

  // Create random generators
  auto rngRay      = RandomGenerator(config.ray_rng_seed);
  auto rngMaterial = RandomGenerator(config.material_rng_seed);

  // Create camera
  auto camera      = Camera(config_ptr);
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  // Create render context
  RenderContext ctx(&scene, &config, &rngRay, &rngMaterial);

  // Render with ImageSOA
  {
    std::cout << "Rendering with ImageSOA..." << '\n';
    ImageSOA imageSoa(imageWidth, imageHeight);
    renderImage(imageSoa, camera, ctx);
    if (!imageSoa.write_to_ppm(FilepathOut)) {
      std::cerr << "Error writing ImageSOA to .ppm file\n";
    }
  }

  return 0;
}
