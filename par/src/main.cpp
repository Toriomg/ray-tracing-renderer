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

  // Buffer intermedio de doubles
  struct RawBuffer {
    std::vector<double> r_data, g_data, b_data;
    size_t width_, height_;

    RawBuffer(size_t w, size_t h)
        : r_data(w * h, 0.0), g_data(w * h, 0.0), b_data(w * h, 0.0), width_(w), height_(h) { }

    [[nodiscard]] size_t indice(size_t r, size_t c) const { return r * width_ + c; }

    void set_pixel(size_t i, Color const & c, double) {
      r_data[i] = c.x;
      g_data[i] = c.y;
      b_data[i] = c.z;
    }
  };

  void printUsage(std::string const & name) {
    std::cerr << "Usage: " << name << " <scene> <config> <output> [options]\n"
              << "Options:\n"
              << "  --render-part <auto|simple|static|affinity>\n"
              << "  --render-grain <size>\n"
              << "  --image-part <auto|simple|static|affinity>\n"
              << "  --image-grain <size>\n"
              << "  --threads <num>\n";
  }

  PartitionerType parsePart(std::string const & s) {
    if (s == "auto") {
      return PartitionerType::Auto;
    }
    if (s == "simple") {
      return PartitionerType::Simple;
    }
    if (s == "static") {
      return PartitionerType::Static;
    }
    if (s == "affinity") {
      return PartitionerType::Affinity;
    }
    return PartitionerType::Auto;
  }

  void parseParallelSettings(std::vector<std::string> const & args, ParallelSettings & ren,
                             ParallelSettings & img) {
    for (size_t i = 4; i < args.size(); ++i) {
      if (args[i] == "--render-part" and i + 1 < args.size()) {
        ren.type = parsePart(args[++i]);
      } else if (args[i] == "--render-grain" and i + 1 < args.size()) {
        ren.grainSize = std::stoull(args[++i]);
      } else if (args[i] == "--image-part" and i + 1 < args.size()) {
        img.type = parsePart(args[++i]);
      } else if (args[i] == "--image-grain" and i + 1 < args.size()) {
        img.grainSize = std::stoull(args[++i]);
      } else if (args[i] == "--threads" and i + 1 < args.size()) {
        ren.maxThreads = std::stoi(args[++i]);
        img.maxThreads = ren.maxThreads;
      }
    }
  }

  std::optional<AppResources> loadResources(std::string const & s_path,
                                            std::string const & c_path) {
    auto c = loadConfigFromFile(c_path);
    if (!c) {
      return std::nullopt;
    }
    auto s = loadSceneFromFile(s_path);
    if (!s) {
      return std::nullopt;
    }
    auto rng = std::make_unique<ParallelRNGManager>((unsigned int) c->ray_rng_seed,
                                                    (unsigned int) c->material_rng_seed);
    return AppResources{std::move(*c), std::move(*s), std::move(rng)};
  }

  void runPipeline(AppResources & res, ParallelSettings const & img_set,
                   std::string const & output_file) {
    Camera cam(res.config);
    auto w = static_cast<size_t>(cam.ProjWindow.imageWidth);
    auto h = static_cast<size_t>(cam.ProjWindow.imageHeight);

    // RenderContext solo acepta 3 argumentos (sin ParallelSettings)
    RenderContext ctx(&res.scene, &res.config, res.rng_manager.get());

    std::cout << "Stage 1: Rendering to RawBuffer (Doubles)...\n";
    RawBuffer raw(w, h);
    renderImage(raw, cam, ctx);

    std::cout << "Stage 2: Processing Image with TBB (Settings: " << (int) img_set.type << ")...\n";
    ImagePar img(w, h);

    // Usar RGBInputData y pasar &img_set (puntero)
    RGBInputData input_data{&raw.r_data, &raw.g_data, &raw.b_data};
    img.fill_from_double(input_data, res.config.gamma, &img_set);

    if (img.write_to_ppm(output_file)) {
      std::cout << "Image written to " << output_file << "\n";
    }
  }

}  // namespace

int main(int argc, char * argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() < 4) {
    printUsage(args[0]);
    return 1;
  }

  ParallelSettings ren_set, img_set;
  parseParallelSettings(args, ren_set, img_set);

  std::unique_ptr<tbb::global_control> gc;
  if (ren_set.maxThreads > 0) {
    gc = std::make_unique<tbb::global_control>(tbb::global_control::max_allowed_parallelism,
                                               (size_t) ren_set.maxThreads);
  }

  auto res = loadResources(args[1], args[2]);
  if (!res) {
    return 1;
  }

  runPipeline(*res, img_set, args[3]);
  return 0;
}
