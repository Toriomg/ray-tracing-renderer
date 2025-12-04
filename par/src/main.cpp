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
#include <memory>
#include <optional>
#include <string>
#include <tbb/global_control.h>
#include <vector>

namespace {

  struct AppResources {
    ConfigSettings config;
    SceneSettings scene;
    std::unique_ptr<ParallelRNGManager> rng_manager;
  };

  // RawBuffer: Intermediate structure to store raw double values from rendering
  // Compatible with renderImage interface but stores raw colors instead of processing them
  struct RawBuffer {
    std::vector<double> r_data;
    std::vector<double> g_data;
    std::vector<double> b_data;
    size_t width_;
    size_t height_;

    RawBuffer(size_t width, size_t height)
        : r_data(width * height, 0.0), g_data(width * height, 0.0), b_data(width * height, 0.0),
          width_(width), height_(height) { }

    // Interface compatible with renderImage template requirements
    [[nodiscard]] size_t indice(size_t row, size_t col) const { return row * width_ + col; }

    // Store raw color values without gamma correction
    void set_pixel(size_t index, Color const & color, double /*gamma*/) {
      r_data[index] = color.x;
      g_data[index] = color.y;
      b_data[index] = color.z;
    }
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

    // Stage 1: Render to raw buffer (stores doubles, no gamma correction)
    std::cout << "Stage 1: Rendering to raw buffer (sequential)...\n";
    RawBuffer raw_buffer(imageWidth, imageHeight);
    renderImage(raw_buffer, camera, ctx);

    // Stage 2: Process raw data with parallel image processing
    std::cout << "Stage 2: Processing image with TBB (parallel)...\n";
    ImagePar image(imageWidth, imageHeight);
    RGBInputData input_data{&raw_buffer.r_data, &raw_buffer.g_data, &raw_buffer.b_data};
    image.fill_from_double(input_data, resources.config.gamma, &imageSettings);

    // Write final output
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
