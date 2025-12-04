#include "../../common/include/camera.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/rendering_engine.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random_par.hpp"
#include "image_par.hpp"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <tbb/global_control.h>
#include <vector>

/*
std::string const FilepathScene  = "/workspace/res/scene_scripts/scene3example.txt";
std::string const FilepathConfig = "/workspace/res/config_scripts/config3example.txt";
std::string const FilepathOut    = "/workspace/outputImagePar.ppm";
*/

namespace {

  void parseParallelSettings(std::vector<std::string> const & args, ParallelSettings & settings) {
    // Parse optional arguments starting from index 4 (after mandatory args)
    for (size_t i = 4; i < args.size(); ++i) {
      if (args[i] == "--partitioner" and i + 1 < args.size()) {
        std::string const & part_type = args[++i];
        if (part_type == "auto") {
          settings.type = PartitionerType::Auto;
        } else if (part_type == "simple") {
          settings.type = PartitionerType::Simple;
        } else if (part_type == "static") {
          settings.type = PartitionerType::Static;
        } else if (part_type == "affinity") {
          settings.type = PartitionerType::Affinity;
        } else {
          std::cerr << "Unknown partitioner type: " << part_type << "\n";
          std::exit(1);
        }
      } else if (args[i] == "--grain" and i + 1 < args.size()) {
        settings.grainSize = std::stoull(args[++i]);
      } else if (args[i] == "--threads" and i + 1 < args.size()) {
        settings.maxThreads = std::stoi(args[++i]);
      } else {
        std::cerr << "Unknown argument: " << args[i] << "\n";
        std::exit(1);
      }
    }
  }

}  // namespace

int main(int argc, char * argv[]) {
  std::vector<std::string> const args(argv, argv + argc);
  if (args.size() < 4) {
    std::cerr << "Usage: " << args[0] << " <scene_file> <config_file> <output_file> [options]\n";
    std::cerr << "Example: " << args[0] << " res/scene.txt res/config.txt output.ppm\n";
    std::cerr << "Options:\n";
    std::cerr << "  --partitioner <auto|simple|static|affinity>\n";
    std::cerr << "  --grain <size>\n";
    std::cerr << "  --threads <num>\n";
    return 1;
  }

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(args[2]);
  if (!config_opt) {
    std::cerr << "Aborting due to configuration file error.\n";
    return 1;
  }
  ConfigSettings const & config = *config_opt;

  // Parse parallel settings from CLI arguments
  ParallelSettings par_settings;
  parseParallelSettings(args, par_settings);

  // Initialize TBB thread limit if specified
  std::optional<tbb::global_control> global_control;
  if (par_settings.maxThreads > 0) {
    global_control.emplace(tbb::global_control::max_allowed_parallelism,
                           static_cast<size_t>(par_settings.maxThreads));
    std::cout << "Limiting TBB to " << par_settings.maxThreads << " threads\n";
  }

  std::optional<SceneSettings> scene_opt = loadSceneFromFile(args[1]);
  if (!scene_opt) {
    std::cerr << "Aborting due to scene file error.\n";
    return 1;
  }
  SceneSettings & scene = *scene_opt;

  // Creamos el generador random paralelizable
  ParallelRNGManager rng_manager(static_cast<unsigned int>(config.ray_rng_seed),
                                 static_cast<unsigned int>(config.material_rng_seed));

  // Create camera
  auto camera      = Camera(config);
  auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
  auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);
  RenderContext ctx(&scene, &config, &rng_manager, &par_settings);
  {
    std::cout << "Rendering with ImagePar..." << '\n';
    ImagePar imageSoa(imageWidth, imageHeight);
    renderImage(imageSoa, camera, ctx);
    if (!imageSoa.write_to_ppm(args[3])) {
      std::cerr << "Error writing ImagePar to .ppm file\n";
    }
  }
  return 0;
}
