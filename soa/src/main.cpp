#include "../../common/include/config_parser.hpp"
#include "../../common/include/rendering_engine.hpp"
#include "../../common/include/scene_parser.hpp"
#include "image_soa.hpp"
#include <iostream>
#include <string>


/*
std::string const FilepathScene  = "/workspace/res/scene_scripts/scene3example.txt";
std::string const FilepathConfig = "/workspace/res/config_scripts/config3example.txt";
std::string const FilepathOut    = "/workspace/outputImageSOA.ppm";
*/

int main(int argc, char * argv[]) {
  std::vector<std::string> const args(argv, argv + argc);
  if (args.size() != 4) {
    std::cerr << "Usage: " << args[0] << " <scene_file> <config_file> <output_file>\n";
    std::cerr << "Example: " << args[0] << " res/scene.txt res/config.txt output.ppm\n";
    return 1;
  }

  // Load configuration and scene
  SceneSettings scene   = loadSceneFromFile(args[1]);
  ConfigSettings config = loadConfigFromFile(args[2]);

  // Create random generators
  auto rngRay      = RandomGenerator(config.ray_rng_seed);
  auto rngMaterial = RandomGenerator(config.material_rng_seed);

  // Create camera
  auto camera      = Camera(config);
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

  // Create render context
  RenderContext ctx(&scene, &config, &rngRay, &rngMaterial);

  // Render with ImageSOA
  {
    std::cout << "Rendering with ImageSOA..." << '\n';
    ImageSOA imageSoa(imageWidth, imageHeight);
    renderImage(imageSoa, camera, ctx);
    if (!imageSoa.write_to_ppm(args[3])) {
      std::cerr << "Error writing ImageSOA to .ppm file\n";
    }
  }

  return 0;
}
