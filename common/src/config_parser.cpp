#include "config_parser.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include <cerrno>
#include <charconv>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

  void trimWhitespace(std::string_view & str) {
    while (!str.empty() and (std::isspace(static_cast<unsigned char>(str.front())) != 0)) {
      str.remove_prefix(1);
    }
    while (!str.empty() and (std::isspace(static_cast<unsigned char>(str.back())) != 0)) {
      str.remove_suffix(1);
    }
  }

  std::vector<std::string_view> tokenizeLine(std::string_view line) {
    std::vector<std::string_view> tokens;

    while (!line.empty()) {
      trimWhitespace(line);
      if (line.empty()) {
        break;
      }

      std::size_t const pos = line.find(' ');
      if (pos == std::string_view::npos) {
        tokens.push_back(line);
        break;
      }

      tokens.push_back(line.substr(0, pos));
      line.remove_prefix(pos);
    }

    return tokens;
  }

  bool parseFloat(std::string_view token, float & value) {
    if (token.empty()) {
      return false;
    }

    char const * const start = token.data();
    char const * const end   = token.data() + token.size();

    auto const result = std::from_chars(start, end, value);
    return result.ec == std::errc() and result.ptr == end;
  }

  bool parseInt(std::string_view token, int & value) {
    if (token.empty()) {
      return false;
    }

    char const * const start = token.data();
    char const * const end   = token.data() + token.size();

    auto const result = std::from_chars(start, end, value);
    return result.ec == std::errc() and result.ptr == end;
  }

  bool parseUnsignedLong(std::string_view token, unsigned long & value) {
    if (token.empty()) {
      return false;
    }

    char const * const start = token.data();
    char const * const end   = token.data() + token.size();

    auto const result = std::from_chars(start, end, value);
    return result.ec == std::errc() and result.ptr == end;
  }

  bool parseUnsignedInt(std::string_view token, unsigned int & value) {
    if (token.empty()) {
      return false;
    }

    char const * const start = token.data();
    char const * const end   = token.data() + token.size();

    auto const result = std::from_chars(start, end, value);
    return result.ec == std::errc() and result.ptr == end;
  }

  bool validateColorComponents(float r, float g, float b) {
    return r >= 0.0F and r <= 1.0F and g >= 0.0F and g <= 1.0F and b >= 0.0F and b <= 1.0F;
  }

  bool parseAspectRatio(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 3) {
      std::cerr << "Error: aspect_ratio requires 2 parameters (width height), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    unsigned int width  = 0;
    unsigned int height = 0;

    if (!parseUnsignedInt(tokens[1], width) or !parseUnsignedInt(tokens[2], height)) {
      std::cerr << "Error: invalid aspect ratio values\n";
      return false;
    }

    if (width <= 0 or height <= 0) {
      std::cerr << "Error: aspect ratio values must be positive\n";
      return false;
    }

    config.aspect_ratio = {width, height};
    return true;
  }

  bool parseImageWidth(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: image_width requires 1 parameter, got " << tokens.size() - 1 << "\n";
      return false;
    }

    int width = 0;
    if (!parseInt(tokens[1], width)) {
      std::cerr << "Error: invalid image width value\n";
      return false;
    }

    if (width <= 0) {
      std::cerr << "Error: image width must be positive\n";
      return false;
    }

    config.image_width = width;
    return true;
  }

  bool parseGamma(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: gamma requires 1 parameter, got " << tokens.size() - 1 << "\n";
      return false;
    }

    float gamma = 0.0F;
    if (!parseFloat(tokens[1], gamma)) {
      std::cerr << "Error: invalid gamma value\n";
      return false;
    }

    config.gamma = gamma;
    return true;
  }

  bool parseCameraPosition(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 4) {
      std::cerr << "Error: camera_position requires 3 parameters (x y z), got " << tokens.size() - 1
                << "\n";
      return false;
    }

    float x = 0.0F, y = 0.0F, z = 0.0F;
    if (!parseFloat(tokens[1], x) or !parseFloat(tokens[2], y) or !parseFloat(tokens[3], z)) {
      std::cerr << "Error: invalid camera position values\n";
      return false;
    }

    config.camera_pos = Point3(x, y, z);
    return true;
  }

  bool parseCameraTarget(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 4) {
      std::cerr << "Error: camera_target requires 3 parameters (x y z), got " << tokens.size() - 1
                << "\n";
      return false;
    }

    float x = 0.0F, y = 0.0F, z = 0.0F;
    if (!parseFloat(tokens[1], x) or !parseFloat(tokens[2], y) or !parseFloat(tokens[3], z)) {
      std::cerr << "Error: invalid camera target values\n";
      return false;
    }

    config.camera_target = Point3(x, y, z);
    return true;
  }

  bool parseCameraNorth(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 4) {
      std::cerr << "Error: camera_north requires 3 parameters (x y z), got " << tokens.size() - 1
                << "\n";
      return false;
    }

    float x = 0.0F, y = 0.0F, z = 0.0F;
    if (!parseFloat(tokens[1], x) or !parseFloat(tokens[2], y) or !parseFloat(tokens[3], z)) {
      std::cerr << "Error: invalid camera north values\n";
      return false;
    }

    config.camera_north = Vec3(x, y, z);
    return true;
  }

  bool parseFieldOfView(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: field_of_view requires 1 parameter, got " << tokens.size() - 1 << "\n";
      return false;
    }

    float fov = 0.0F;
    if (!parseFloat(tokens[1], fov)) {
      std::cerr << "Error: invalid field of view value\n";
      return false;
    }

    if (fov <= 0.0F or fov >= 180.0F) {
      std::cerr << "Error: field of view must be between 0 and 180 degrees\n";
      return false;
    }

    config.field_of_view = fov;
    return true;
  }

  bool parseSamplesPerPixel(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: samples_per_pixel requires 1 parameter, got " << tokens.size() - 1
                << "\n";
      return false;
    }

    int samples = 0;
    if (!parseInt(tokens[1], samples)) {
      std::cerr << "Error: invalid samples per pixel value\n";
      return false;
    }

    if (samples <= 0) {
      std::cerr << "Error: samples per pixel must be positive\n";
      return false;
    }

    config.samples_per_pixel = samples;
    return true;
  }

  bool parseMaxDepth(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: max_depth requires 1 parameter, got " << tokens.size() - 1 << "\n";
      return false;
    }

    int depth = 0;
    if (!parseInt(tokens[1], depth)) {
      std::cerr << "Error: invalid max depth value\n";
      return false;
    }

    if (depth <= 0) {
      std::cerr << "Error: max depth must be positive\n";
      return false;
    }

    config.max_depth = depth;
    return true;
  }

  bool parseMaterialRngSeed(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: material_rng_seed requires 1 parameter, got " << tokens.size() - 1
                << "\n";
      return false;
    }

    unsigned long seed = 0;
    if (!parseUnsignedLong(tokens[1], seed)) {
      std::cerr << "Error: invalid material RNG seed value\n";
      return false;
    }

    if (seed <= 0) {
      std::cerr << "Error: material RNG seed must be positive\n";
      return false;
    }

    config.material_rng_seed = seed;
    return true;
  }

  bool parseRayRngSeed(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() != 2) {
      std::cerr << "Error: ray_rng_seed requires 1 parameter, got " << tokens.size() - 1 << "\n";
      return false;
    }

    unsigned long seed = 0;
    if (!parseUnsignedLong(tokens[1], seed)) {
      std::cerr << "Error: invalid ray RNG seed value\n";
      return false;
    }

    if (seed <= 0) {
      std::cerr << "Error: ray RNG seed must be positive\n";
      return false;
    }

    config.ray_rng_seed = seed;
    return true;
  }

  bool parseBackgroundDarkColor(std::vector<std::string_view> const & tokens,
                                ConfigSettings & config) {
    if (tokens.size() != 4) {
      std::cerr << "Error: background_dark_color requires 3 parameters (r g b), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    float r = 0.0F, g = 0.0F, b = 0.0F;
    if (!parseFloat(tokens[1], r) or !parseFloat(tokens[2], g) or !parseFloat(tokens[3], b)) {
      std::cerr << "Error: invalid background dark color values\n";
      return false;
    }

    if (!validateColorComponents(r, g, b)) {
      std::cerr << "Error: background dark color values must be in range [0, 1]\n";
      return false;
    }

    config.background_dark_color = Color(r, g, b);
    return true;
  }

  bool parseBackgroundLightColor(std::vector<std::string_view> const & tokens,
                                 ConfigSettings & config) {
    if (tokens.size() != 4) {
      std::cerr << "Error: background_light_color requires 3 parameters (r g b), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    float r = 0.0F, g = 0.0F, b = 0.0F;
    if (!parseFloat(tokens[1], r) or !parseFloat(tokens[2], g) or !parseFloat(tokens[3], b)) {
      std::cerr << "Error: invalid background light color values\n";
      return false;
    }

    if (!validateColorComponents(r, g, b)) {
      std::cerr << "Error: background light color values must be in range [0, 1]\n";
      return false;
    }

    config.background_light_color = Color(r, g, b);
    return true;
  }

  bool processLine(std::string_view line, ConfigSettings & config) {
    trimWhitespace(line);
    if (line.empty() or line.front() == '#') {
      return true;
    }

    std::vector<std::string_view> const tokens = tokenizeLine(line);
    if (tokens.empty()) {
      return true;
    }

    static std::unordered_map<std::string_view,
                              std::function<bool(std::vector<std::string_view> const &,
                                                 ConfigSettings &)>> const commands = {
      {          "aspect_ratio:",          parseAspectRatio},
      {           "image_width:",           parseImageWidth},
      {                 "gamma:",                parseGamma},
      {       "camera_position:",       parseCameraPosition},
      {         "camera_target:",         parseCameraTarget},
      {          "camera_north:",          parseCameraNorth},
      {         "field_of_view:",          parseFieldOfView},
      {     "samples_per_pixel:",      parseSamplesPerPixel},
      {             "max_depth:",             parseMaxDepth},
      {     "material_rng_seed:",      parseMaterialRngSeed},
      {          "ray_rng_seed:",           parseRayRngSeed},
      { "background_dark_color:",  parseBackgroundDarkColor},
      {"background_light_color:", parseBackgroundLightColor}
    };

    auto it = commands.find(tokens[0]);
    if (it != commands.end()) {
      return it->second(tokens, config);
    }

    std::cerr << "Error: unknown command '" << tokens[0] << "'\n";
    return false;
  }

}  // namespace

ConfigSettings loadConfigFromFile(std::string const & filename) {
  ConfigSettings config;

  // Default values
  config.aspect_ratio           = Constants::AspectRatio;
  config.image_width            = Constants::ImageWidth;
  config.gamma                  = Constants::Gamma;
  config.camera_pos             = Constants::CameraPosition;
  config.camera_target          = Constants::CameraTarget;
  config.camera_north           = Constants::CameraNorth;
  config.field_of_view          = Constants::FOV;
  config.samples_per_pixel      = Constants::SamplesPerPixel;
  config.max_depth              = Constants::MaxDepth;
  config.material_rng_seed      = Constants::RNGSeedMaterial;
  config.ray_rng_seed           = Constants::RNGSeedRay;
  config.background_dark_color  = Constants::ColorBackgroundDark;
  config.background_light_color = Constants::ColorBackGroundLight;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open config file '" << filename << "'\n";
    return config;
  }

  std::string line;
  std::size_t lineNumber = 0;

  while (std::getline(file, line)) {
    ++lineNumber;
    if (!processLine(line, config)) {
      std::cerr << "Error parsing line " << lineNumber << ": " << line << "\n";
    }
  }

  return config;
}
