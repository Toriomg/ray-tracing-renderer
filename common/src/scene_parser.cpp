#include "../include/scene_parser.hpp"
#include "../include/dataStructs/aabb.hpp"
#include "../include/dataStructs/bvh.hpp"
#include "../include/dataStructs/material.hpp"
#include "../include/dataStructs/settings_structs.hpp"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

  enum class ParseError {
    None,
    InvalidParameters,
    ExtraData,
    DuplicateMaterialName,
    MaterialNotFound
  };

  struct ParseResult {
    ParseError error_code;
    std::string data;

    explicit ParseResult(ParseError code) : error_code(code) { }

    ParseResult(ParseError code, std::string d) : error_code(code), data(std::move(d)) { }
  };

  // Forward declarations
  ParseResult parseMatteMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseMetalMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseRefractiveMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseSphere(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseCylinder(std::vector<std::string_view> const &, SceneSettings &);

  auto const & getCommands() {
    using ParserFunc =
        std::function<ParseResult(std::vector<std::string_view> const &, SceneSettings &)>;
    static std::unordered_map<std::string_view, ParserFunc> const COMMANDS = {
      {     "matte:",      parseMatteMaterial},
      {     "metal:",      parseMetalMaterial},
      {"refractive:", parseRefractiveMaterial},
      {    "sphere:",             parseSphere},
      {  "cylinder:",           parseCylinder}
    };
    return COMMANDS;
  }

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
      std::size_t const pos = line.find_first_of(" \t");
      if (pos == std::string_view::npos) {
        tokens.push_back(line);
        break;
      }
      tokens.push_back(line.substr(0, pos));
      line.remove_prefix(pos);
    }
    return tokens;
  }

  std::string join_tokens(std::vector<std::string_view> const & tokens, std::size_t start) {
    if (start >= tokens.size()) {
      return "";
    }
    return std::accumulate(
        std::next(tokens.begin(), static_cast<long>(start + 1)), tokens.end(),
        std::string(tokens[start]),
        [](std::string const & a, std::string_view b) { return a + " " + std::string(b); });
  }

  [[nodiscard]] bool parsedouble(std::string_view token, double & value) {
    if (token.empty()) {
      return false;
    }
    if (token.front() == '+') {
      token.remove_prefix(1);
    }
    auto const [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
    if (ec != std::errc() or ptr != token.data() + token.size()) {
      return false;
    }
    return !(std::isnan(value) or std::isinf(value));
  }

  int findMaterialIndex(std::string_view materialName, SceneSettings const & scene) {
    for (std::size_t i = 0; i < scene.materialNames.size(); ++i) {
      if (scene.materialNames[i] == materialName) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  ParseResult parseMatteMaterial(std::vector<std::string_view> const & tokens, SceneSettings & s) {
    if (tokens.size() < 5) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 5) {
      return {ParseError::ExtraData, join_tokens(tokens, 5)};
    }
    if (findMaterialIndex(tokens[1], s) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double r = 0, g = 0, b = 0;
    if (!parsedouble(tokens[2], r) or
        !parsedouble(tokens[3], g) or
        !parsedouble(tokens[4], b) or
        r < 0 or
        r > 1 or
        g < 0 or
        g > 1 or
        b < 0 or
        b > 1)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const idx = s.matte.r.size();
    s.matte.r.push_back(r);
    s.matte.g.push_back(g);
    s.matte.b.push_back(b);
    s.materialTable.push_back({MaterialType::MATTE, static_cast<unsigned int>(idx)});
    s.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseMetalMaterial(std::vector<std::string_view> const & tokens, SceneSettings & s) {
    if (tokens.size() < 6) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 6) {
      return {ParseError::ExtraData, join_tokens(tokens, 6)};
    }
    if (findMaterialIndex(tokens[1], s) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double r = 0, g = 0, b = 0, diff = 0;
    if (!parsedouble(tokens[2], r) or
        !parsedouble(tokens[3], g) or
        !parsedouble(tokens[4], b) or
        !parsedouble(tokens[5], diff) or
        r < 0 or
        r > 1 or
        g < 0 or
        g > 1 or
        b < 0 or
        b > 1 or
        diff < 0)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const idx = s.metal.r.size();
    s.metal.r.push_back(r);
    s.metal.g.push_back(g);
    s.metal.b.push_back(b);
    s.metal.diffusion.push_back(diff);
    s.materialTable.push_back({MaterialType::METAL, static_cast<unsigned int>(idx)});
    s.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseRefractiveMaterial(std::vector<std::string_view> const & tokens,
                                      SceneSettings & s) {
    if (tokens.size() < 3) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 3) {
      return {ParseError::ExtraData, join_tokens(tokens, 3)};
    }
    if (findMaterialIndex(tokens[1], s) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double ior = 0;
    if (!parsedouble(tokens[2], ior) or ior <= 0) {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const idx = s.refractive.ior.size();
    s.refractive.ior.push_back(ior);
    s.materialTable.push_back({MaterialType::REFRACTIVE, static_cast<unsigned int>(idx)});
    s.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseSphere(std::vector<std::string_view> const & tokens, SceneSettings & s) {
    if (tokens.size() < 6) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 6) {
      return {ParseError::ExtraData, join_tokens(tokens, 6)};
    }

    double x = 0, y = 0, z = 0, r = 0;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], r) or
        r <= 0)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    int const matIdx = findMaterialIndex(tokens[5], s);
    if (matIdx == -1) {
      return {ParseError::MaterialNotFound, std::string(tokens[5])};
    }

    s.spheres.x.push_back(x);
    s.spheres.y.push_back(y);
    s.spheres.z.push_back(z);
    s.spheres.r.push_back(r);
    s.spheres.materialIndex.push_back(static_cast<unsigned int>(matIdx));
    s.spheres.aabbs.push_back(AABB::from_sphere({x, y, z}, r));
    return ParseResult(ParseError::None);
  }

  ParseResult parseCylinder(std::vector<std::string_view> const & tokens, SceneSettings & s) {
    if (tokens.size() < 9) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 9) {
      return {ParseError::ExtraData, join_tokens(tokens, 9)};
    }

    double x = 0, y = 0, z = 0, r = 0, vx = 0, vy = 0, vz = 0;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], r) or
        !parsedouble(tokens[5], vx) or
        !parsedouble(tokens[6], vy) or
        !parsedouble(tokens[7], vz) or
        r <= 0 or
        (vx == 0 and vy == 0 and vz == 0))
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    int const matIdx = findMaterialIndex(tokens[8], s);
    if (matIdx == -1) {
      return {ParseError::MaterialNotFound, std::string(tokens[8])};
    }

    double const len = std::sqrt(vx * vx + vy * vy + vz * vz);
    s.cylinders.addCentre(x, y, z);
    s.cylinders.addAxis(vx, vy, vz);
    s.cylinders.r.push_back(r);
    s.cylinders.materialIndex.push_back(matIdx);
    s.cylinders.addAABB(AABB::from_cylinder({x, y, z}, {vx, vy, vz}, r, len));
    return ParseResult(ParseError::None);
  }

  bool logSceneParseResult(ParseResult const & result, std::string_view key,
                           std::string_view original_line) {
    if (result.error_code == ParseError::None) {
      return true;
    }

    std::string_view name = key;
    name.remove_suffix(1);

    if (result.error_code == ParseError::InvalidParameters) {
      std::cerr << "Error: Invalid " << name << " parameters\n";
    } else if (result.error_code == ParseError::ExtraData) {
      std::cerr << "Error: Extra data for [" << key << "]: \"" << result.data << "\"\n";
    } else if (result.error_code == ParseError::DuplicateMaterialName) {
      std::cerr << "Error: Duplicate material: [" << result.data << "]\n";
    } else if (result.error_code == ParseError::MaterialNotFound) {
      std::cerr << "Error: Material not found: [" << result.data << "]\n";
    }

    std::cerr << "Line: \"" << original_line << "\"\n";
    return false;
  }

  bool processLine(std::string_view original_line, SceneSettings & scene) {
    auto line = original_line;
    trimWhitespace(line);
    if (line.empty() or line.front() == '#') {
      return true;
    }

    std::vector<std::string_view> const tokens = tokenizeLine(line);
    if (tokens.empty()) {
      return true;
    }

    auto const & commands = getCommands();
    auto const & key      = tokens[0];
    auto it               = commands.find(key);

    if (it == commands.end()) {
      std::string_view name = key;
      if (!key.empty() and key.back() == ':') {
        name.remove_suffix(1);
      }
      std::cerr << "Error: Unknown scene entity: " << name << "\n";
      return false;
    }

    ParseResult const result = it->second(tokens, scene);
    return logSceneParseResult(result, key, original_line);
  }

}  // namespace

std::optional<SceneSettings> loadSceneFromFile(std::string const & filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open scene file '" << filename << "'\n";
    return std::nullopt;
  }

  SceneSettings scene;
  std::string line;
  while (std::getline(file, line)) {
    if (!processLine(line, scene)) {
      return std::nullopt;
    }
  }

  scene.bvh.build(scene.spheres, scene.cylinders);

  return scene;
}
