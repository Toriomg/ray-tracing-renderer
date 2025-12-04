#include "../../common/include/camera.hpp"
#include "../../common/include/config_parser.hpp"
#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/rendering_engine.hpp"
#include "../../common/include/scene_parser.hpp"
#include "../../common/include/utilities/random_par.hpp"
#include "image_par.hpp"
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

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

  // Stage 1: Sequential rendering to raw buffer
  void render_to_raw_buffer(RawBuffer & buffer, Camera & camera, RenderContext & ctx) {
    std::cout << "Stage 1: Rendering to raw buffer (sequential)...\n";
    renderImage(buffer, camera, ctx);
  }

  // Stage 2: Parallel image processing with TBB
  void process_image_parallel(ImagePar & image, RawBuffer const & raw_buffer, double gamma) {
    std::cout << "Stage 2: Processing image with TBB (parallel)...\n";
    image.fill_from_double(raw_buffer.r_data, raw_buffer.g_data, raw_buffer.b_data, gamma);
  }

  // Load and validate configuration file
  std::optional<ConfigSettings> load_config(std::string const & config_path) {
    std::optional<ConfigSettings> config_opt = loadConfigFromFile(config_path);
    if (!config_opt) {
      std::cerr << "Aborting due to configuration file error.\n";
    }
    return config_opt;
  }

  // Load and validate scene file
  std::optional<SceneSettings> load_scene(std::string const & scene_path) {
    std::optional<SceneSettings> scene_opt = loadSceneFromFile(scene_path);
    if (!scene_opt) {
      std::cerr << "Aborting due to scene file error.\n";
    }
    return scene_opt;
  }

  // Main rendering pipeline: loads configuration, renders to raw buffer, then processes with TBB
  int run_rendering_pipeline(std::string const & scene_path, std::string const & config_path,
                             std::string const & output_path) {
    // Load configuration and scene
    std::optional<ConfigSettings> config_opt = load_config(config_path);
    if (!config_opt) {
      return 1;
    }
    ConfigSettings const & config = *config_opt;

    std::optional<SceneSettings> scene_opt = load_scene(scene_path);
    if (!scene_opt) {
      return 1;
    }
    SceneSettings & scene = *scene_opt;

    // Initialize parallel RNG manager
    ParallelRNGManager rng_manager(static_cast<unsigned int>(config.ray_rng_seed),
                                   static_cast<unsigned int>(config.material_rng_seed));

    // Setup camera and image dimensions
    auto camera      = Camera(config);
    auto imageWidth  = static_cast<size_t>(camera.ProjWindow.imageWidth);
    auto imageHeight = static_cast<size_t>(camera.ProjWindow.imageHeight);
    RenderContext ctx(&scene, &config, &rng_manager);

    // Execute two-stage pipeline
    RawBuffer raw_buffer(imageWidth, imageHeight);
    render_to_raw_buffer(raw_buffer, camera, ctx);

    ImagePar imageSoa(imageWidth, imageHeight);
    process_image_parallel(imageSoa, raw_buffer, config.gamma);

    // Write final output
    if (!imageSoa.write_to_ppm(output_path)) {
      std::cerr << "Error writing image to .ppm file\n";
      return 1;
    }
    std::cout << "Image written to " << output_path << "\n";

    return 0;
  }

}  // namespace

int main(int argc, char * argv[]) {
  std::vector<std::string> const args(argv, argv + argc);
  if (args.size() != 4) {
    std::cerr << "Usage: " << args[0] << " <scene_file> <config_file> <output_file>\n";
    std::cerr << "Example: " << args[0] << " res/scene.txt res/config.txt output.ppm\n";
    return 1;
  }

  return run_rendering_pipeline(args[1], args[2], args[3]);
}
