#include "../../common/include/camera.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/rendering_engine.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random.hpp"
#include "image_soa.hpp"
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(args[2]);
  if (!config_opt) {
    std::cerr << "Aborting due to configuration file error.\n";
    return 1;
  }
  ConfigSettings const & config = *config_opt;

  std::optional<SceneSettings> scene_opt = loadSceneFromFile(args[1]);
  if (!scene_opt) {
    std::cerr << "Aborting due to scene file error.\n";
    return 1;
  }
  SceneSettings & scene = *scene_opt;

  // Create random generators
  auto rngRay      = RandomGenerator(config.ray_rng_seed);
  auto rngMaterial = RandomGenerator(config.material_rng_seed);

  // Create camera
  auto camera      = Camera(config);
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);
  RenderContext ctx(&scene, &config, &rngRay, &rngMaterial);
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
