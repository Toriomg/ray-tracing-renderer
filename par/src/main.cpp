#include "../../common/include/camera.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/rendering_engine.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random_par.hpp"
#include "image_par.hpp"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tbb/global_control.h>
#include <utility>
#include <vector>

namespace {

  struct AppResources {
    ConfigSettings config;
    SceneSettings scene;
    std::unique_ptr<ParallelRNGManager> rng_manager;
  };

  void printUsage(std::string const & program_name) {
    std::cerr << "Usage: " << program_name
              << " <scene_file> <config_file> <output_file> [options]\n";
    std::cerr << "Example: " << program_name << " res/scene.txt res/config.txt output.ppm\n";
    std::cerr << "Options:\n";
    std::cerr << "  --partitioner <auto|simple|static|affinity>  Select TBB partitioner type\n";
    std::cerr
        << "  --grain <size>                                Set grain size for partitioning\n";
    std::cerr << "  --threads <num>                               Limit number of TBB threads\n";
  }

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

  std::optional<AppResources> loadAppResources(std::string const & scene_path,
                                               std::string const & config_path) {
    // Load configuration
    std::optional<ConfigSettings> config_opt = loadConfigFromFile(config_path);
    if (!config_opt) {
      std::cerr << "Aborting due to configuration file error.\n";
      return std::nullopt;
    }

    // Load scene
    std::optional<SceneSettings> scene_opt = loadSceneFromFile(scene_path);
    if (!scene_opt) {
      std::cerr << "Aborting due to scene file error.\n";
      return std::nullopt;
    }

    // Initialize RNG manager
    auto rng_manager = std::make_unique<ParallelRNGManager>(
        static_cast<unsigned int>(config_opt->ray_rng_seed),
        static_cast<unsigned int>(config_opt->material_rng_seed));

    // Construct and return resources
    AppResources resources{std::move(*config_opt), std::move(*scene_opt), std::move(rng_manager)};
    return resources;
  }

  void runRenderPipeline(AppResources & resources, ParallelSettings const & par_settings,
                         std::string const & output_file) {
    // Create camera and determine image dimensions
    auto camera      = Camera(resources.config);
    auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
    auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

    // Create render context with parallel settings
    RenderContext ctx(&resources.scene, &resources.config, resources.rng_manager.get(),
                      &par_settings);

    // Render and write output
    std::cout << "Rendering with ImagePar...\n";
    ImagePar image(imageWidth, imageHeight);
    renderImage(image, camera, ctx);

    if (!image.write_to_ppm(output_file)) {
      std::cerr << "Error writing image to .ppm file\n";
      std::exit(1);
    }
    std::cout << "Image written to " << output_file << "\n";
  }

}  // namespace

int main(int argc, char * argv[]) {
  std::vector<std::string> const args(argv, argv + argc);

  // Validate arguments
  if (args.size() < 4) {
    printUsage(args[0]);
    return 1;
  }

  // Parse parallel settings
  ParallelSettings par_settings;
  parseParallelSettings(args, par_settings);

  // Initialize TBB thread limit if specified
  std::unique_ptr<tbb::global_control> global_control;
  if (par_settings.maxThreads > 0) {
    global_control = std::make_unique<tbb::global_control>(
        tbb::global_control::max_allowed_parallelism, static_cast<size_t>(par_settings.maxThreads));
    std::cout << "Limiting TBB to " << par_settings.maxThreads << " threads\n";
  }

  // Load application resources
  auto resources = loadAppResources(args[1], args[2]);
  if (!resources) {
    return 1;
  }

  // Run the render pipeline
  runRenderPipeline(*resources, par_settings, args[3]);

  return 0;
}
