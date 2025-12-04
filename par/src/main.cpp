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
    std::cerr << "  Global:\n";
    std::cerr << "    --threads <num>                             Limit number of TBB threads\n";
    std::cerr << "  Rendering (legacy - applies to both):\n";
    std::cerr << "    --partitioner <auto|simple|static|affinity> Set partitioner for both\n";
    std::cerr << "    --grain <size>                              Set grain size for both\n";
    std::cerr << "  Rendering (specific):\n";
    std::cerr << "    --render-part <auto|simple|static|affinity> Set render partitioner\n";
    std::cerr << "    --render-grain <size>                       Set render grain size\n";
    std::cerr << "  Image Processing (specific):\n";
    std::cerr << "    --image-part <auto|simple|static|affinity>  Set image partitioner\n";
    std::cerr << "    --image-grain <size>                        Set image grain size\n";
  }

  PartitionerType parsePartitionerType(std::string const & part_type) {
    if (part_type == "auto") {
      return PartitionerType::Auto;
    }
    if (part_type == "simple") {
      return PartitionerType::Simple;
    }
    if (part_type == "static") {
      return PartitionerType::Static;
    }
    if (part_type == "affinity") {
      return PartitionerType::Affinity;
    }
    std::cerr << "Unknown partitioner type: " << part_type << "\n";
    std::exit(1);
  }

  bool tryParseLegacyArgs(size_t & i, std::vector<std::string> const & args,
                          ParallelSettings & renderSettings, ParallelSettings & imageSettings) {
    std::string const & arg = args[i];

    if (arg == "--partitioner" and i + 1 < args.size()) {
      PartitionerType const type = parsePartitionerType(args[++i]);
      renderSettings.type        = type;
      imageSettings.type         = type;
      return true;
    }
    if (arg == "--grain" and i + 1 < args.size()) {
      size_t const grain       = std::stoull(args[++i]);
      renderSettings.grainSize = grain;
      imageSettings.grainSize  = grain;
      return true;
    }
    return false;
  }

  bool tryParseRenderArgs(size_t & i, std::vector<std::string> const & args,
                          ParallelSettings & renderSettings) {
    std::string const & arg = args[i];

    if (arg == "--render-part" and i + 1 < args.size()) {
      renderSettings.type = parsePartitionerType(args[++i]);
      return true;
    }
    if (arg == "--render-grain" and i + 1 < args.size()) {
      renderSettings.grainSize = std::stoull(args[++i]);
      return true;
    }
    return false;
  }

  bool tryParseImageArgs(size_t & i, std::vector<std::string> const & args,
                         ParallelSettings & imageSettings) {
    std::string const & arg = args[i];

    if (arg == "--image-part" and i + 1 < args.size()) {
      imageSettings.type = parsePartitionerType(args[++i]);
      return true;
    }
    if (arg == "--image-grain" and i + 1 < args.size()) {
      imageSettings.grainSize = std::stoull(args[++i]);
      return true;
    }
    return false;
  }

  bool tryParseGlobalArgs(size_t & i, std::vector<std::string> const & args,
                          ParallelSettings & renderSettings, ParallelSettings & imageSettings) {
    std::string const & arg = args[i];

    if (arg == "--threads" and i + 1 < args.size()) {
      int const threads         = std::stoi(args[++i]);
      renderSettings.maxThreads = threads;
      imageSettings.maxThreads  = threads;
      return true;
    }
    return false;
  }

  bool parseSingleArg(size_t & i, std::vector<std::string> const & args,
                      ParallelSettings & renderSettings, ParallelSettings & imageSettings) {
    if (tryParseLegacyArgs(i, args, renderSettings, imageSettings)) {
      return true;
    }
    if (tryParseRenderArgs(i, args, renderSettings)) {
      return true;
    }
    if (tryParseImageArgs(i, args, imageSettings)) {
      return true;
    }
    if (tryParseGlobalArgs(i, args, renderSettings, imageSettings)) {
      return true;
    }

    std::cerr << "Unknown argument: " << args[i] << "\n";
    std::exit(1);
  }

  void parseParallelSettings(std::vector<std::string> const & args,
                             ParallelSettings & renderSettings, ParallelSettings & imageSettings) {
    // Parse optional arguments starting from index 4 (after mandatory args)
    for (size_t i = 4; i < args.size(); ++i) {
      parseSingleArg(i, args, renderSettings, imageSettings);
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

  void runRenderPipeline(AppResources & resources, ParallelSettings const & renderSettings,
                         ParallelSettings const & imageSettings, std::string const & output_file) {
    // Create camera and determine image dimensions
    auto camera      = Camera(resources.config);
    auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
    auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);

    // Create render context with render-specific settings
    RenderContext ctx(&resources.scene, &resources.config, resources.rng_manager.get(),
                      &renderSettings);

    // Render and write output
    std::cout << "Rendering with ImagePar...\n";
    ImagePar image(imageWidth, imageHeight);
    renderImage(image, camera, ctx);

    // Note: imageSettings is available for future use with fill_from_double or other
    // image processing operations that may benefit from different parallel configuration
    (void) imageSettings;  // Suppress unused parameter warning

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

  // Parse parallel settings for rendering and image processing
  ParallelSettings render_settings;
  ParallelSettings image_settings;
  parseParallelSettings(args, render_settings, image_settings);

  // Initialize TBB thread limit if specified
  std::unique_ptr<tbb::global_control> global_control;
  if (render_settings.maxThreads > 0) {
    global_control =
        std::make_unique<tbb::global_control>(tbb::global_control::max_allowed_parallelism,
                                              static_cast<size_t>(render_settings.maxThreads));
    std::cout << "Limiting TBB to " << render_settings.maxThreads << " threads\n";
  }

  // Load application resources
  auto resources = loadAppResources(args[1], args[2]);
  if (!resources) {
    return 1;
  }

  // Run the render pipeline with separate settings
  runRenderPipeline(*resources, render_settings, image_settings, args[3]);

  return 0;
}
