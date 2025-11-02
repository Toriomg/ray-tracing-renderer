#include "scene_parser.hpp"
#include "dataStructs/aabb.hpp"
#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"

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

// --- Espacio de nombres anónimo para helpers internos del parser ---
namespace {

  // --- Estructuras y Tipos para el Manejo de Errores ---
  enum class ParseError {
    None,
    InvalidParameters,      // Parámetro inválido (tipo, rango) o insuficiente
    ExtraData,              // Demasiados parámetros
    DuplicateMaterialName,  // El nombre del material ya existe
    MaterialNotFound        // Se referencia un material no definido
  };

  struct ParseResult {
    ParseError error_code;
    std::string data;  // Usado para nombres de material o datos extra

    explicit ParseResult(ParseError code) : error_code(code) { }

    ParseResult(ParseError code, std::string d) : error_code(code), data(std::move(d)) { }
  };

  // --- Prototipos de las funciones de parseo ---
  ParseResult parseMatteMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseMetalMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseRefractiveMaterial(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseSphere(std::vector<std::string_view> const &, SceneSettings &);
  ParseResult parseCylinder(std::vector<std::string_view> const &, SceneSettings &);

  // --- Tabla de Despacho (Command Map) ---
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

  // --- Utilidades y Helpers ---
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

  std::string join_tokens(std::vector<std::string_view> const & tokens, std::size_t start_index) {
    if (start_index >= tokens.size()) {
      return "";
    }
    return std::accumulate(
        std::next(tokens.begin(), static_cast<long>(start_index + 1)), tokens.end(),
        std::string(tokens[start_index]),
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
    if (std::isnan(value) or std::isinf(value)) {
      return false;
    }
    return true;
  }

  int findMaterialIndex(std::string_view materialName, SceneSettings const & scene) {
    for (std::size_t i = 0; i < scene.materialNames.size(); ++i) {
      if (scene.materialNames[i] == materialName) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  // --- Implementaciones de Funciones de Parseo para cada Clave ---

  ParseResult parseMatteMaterial(std::vector<std::string_view> const & tokens,
                                 SceneSettings & scene) {
    if (tokens.size() < 5) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 5) {
      return {ParseError::ExtraData, join_tokens(tokens, 5)};
    }

    if (findMaterialIndex(tokens[1], scene) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double r = 0.0, g = 0.0, b = 0.0;
    if (!parsedouble(tokens[2], r) or
        !parsedouble(tokens[3], g) or
        !parsedouble(tokens[4], b) or
        r < 0.0 or
        r > 1.0 or
        g < 0.0 or
        g > 1.0 or
        b < 0.0 or
        b > 1.0)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const new_idx = scene.matte.r.size();
    scene.matte.r.push_back(r);
    scene.matte.g.push_back(g);
    scene.matte.b.push_back(b);
    scene.materialTable.push_back({MaterialType::MATTE, static_cast<unsigned int>(new_idx)});
    scene.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseMetalMaterial(std::vector<std::string_view> const & tokens,
                                 SceneSettings & scene) {
    if (tokens.size() < 6) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 6) {
      return {ParseError::ExtraData, join_tokens(tokens, 6)};
    }

    if (findMaterialIndex(tokens[1], scene) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double r = 0.0, g = 0.0, b = 0.0, diffusion = 0.0;
    if (!parsedouble(tokens[2], r) or
        !parsedouble(tokens[3], g) or
        !parsedouble(tokens[4], b) or
        !parsedouble(tokens[5], diffusion) or
        r < 0.0 or
        r > 1.0 or
        g < 0.0 or
        g > 1.0 or
        b < 0.0 or
        b > 1.0 or
        diffusion < 0.0)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const new_idx = scene.metal.r.size();
    scene.metal.r.push_back(r);
    scene.metal.g.push_back(g);
    scene.metal.b.push_back(b);
    scene.metal.diffusion.push_back(diffusion);
    scene.materialTable.push_back({MaterialType::METAL, static_cast<unsigned int>(new_idx)});
    scene.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseRefractiveMaterial(std::vector<std::string_view> const & tokens,
                                      SceneSettings & scene) {
    // Comprueba si faltan parámetros
    if (tokens.size() < 3) {
      return ParseResult(ParseError::InvalidParameters);
    }
    // Comprueba si sobran parámetros
    if (tokens.size() > 3) {
      return {ParseError::ExtraData, join_tokens(tokens, 3)};
    }

    if (findMaterialIndex(tokens[1], scene) != -1) {
      return {ParseError::DuplicateMaterialName, std::string(tokens[1])};
    }

    double ior = 0.0;
    if (!parsedouble(tokens[2], ior) or ior <= 0.0) {
      return ParseResult(ParseError::InvalidParameters);
    }

    size_t const new_idx = scene.refractive.ior.size();
    scene.refractive.ior.push_back(ior);
    scene.materialTable.push_back({MaterialType::REFRACTIVE, static_cast<unsigned int>(new_idx)});
    scene.materialNames.emplace_back(tokens[1]);
    return ParseResult(ParseError::None);
  }

  ParseResult parseSphere(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() < 6) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 6) {
      return {ParseError::ExtraData, join_tokens(tokens, 6)};
    }

    double x = 0.0, y = 0.0, z = 0.0, radius = 0.0;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], radius) or
        radius <= 0.0)
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    int const materialIndex = findMaterialIndex(tokens[5], scene);
    if (materialIndex == -1) {
      return {ParseError::MaterialNotFound, std::string(tokens[5])};
    }

    scene.spheres.x.push_back(x);
    scene.spheres.y.push_back(y);
    scene.spheres.z.push_back(z);
    scene.spheres.r.push_back(radius);
    scene.spheres.materialIndex.push_back(static_cast<unsigned int>(materialIndex));
    scene.spheres.aabbs.push_back(AABB::from_sphere({x, y, z}, radius));
    return ParseResult(ParseError::None);
  }

  ParseResult parseCylinder(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() < 9) {
      return ParseResult(ParseError::InvalidParameters);
    }
    if (tokens.size() > 9) {
      return {ParseError::ExtraData, join_tokens(tokens, 9)};
    }

    double x = 0.0, y = 0.0, z = 0.0, radius = 0.0, vx = 0.0, vy = 0.0, vz = 0.0;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], radius) or
        !parsedouble(tokens[5], vx) or
        !parsedouble(tokens[6], vy) or
        !parsedouble(tokens[7], vz) or
        radius <= 0.0 or
        (vx == 0.0 and vy == 0.0 and vz == 0.0))
    {
      return ParseResult(ParseError::InvalidParameters);
    }

    int const materialIndex = findMaterialIndex(tokens[8], scene);
    if (materialIndex == -1) {
      return {ParseError::MaterialNotFound, std::string(tokens[8])};
    }

    double const axisLength = std::sqrt(vx * vx + vy * vy + vz * vz);
    scene.cylinders.addCentre(x, y, z);
    scene.cylinders.addAxis(vx, vy, vz);
    scene.cylinders.r.push_back(radius);
    scene.cylinders.materialIndex.push_back(materialIndex);
    scene.cylinders.addAABB(AABB::from_cylinder({x, y, z}, {vx, vy, vz}, radius, axisLength));
    return ParseResult(ParseError::None);
  }

  // --- Lógica Central de Procesamiento y Errores ---
  // --- Lógica de Reporte de Errores (Extraída de processLine) ---
  bool logSceneParseResult(ParseResult const & result, std::string_view key,
                           std::string_view original_line) {
    if (result.error_code == ParseError::None) {
      return true;
    }

    std::string_view entity_name = key;
    entity_name.remove_suffix(1);  // Quita el ':' para los mensajes

    switch (result.error_code) {
      case ParseError::InvalidParameters:
        std::cerr << "Error: Invalid " << entity_name << " parameters\n";
        break;
      case ParseError::ExtraData:
        std::cerr << "Error: Extra data after configuration value for key: [" << key << "]\n"
                  << "Extra: \"" << result.data << "\"\n";
        break;
      case ParseError::DuplicateMaterialName:
        std::cerr << "Error: Material with name [" << result.data << "] already exists\n";
        break;
      case ParseError::MaterialNotFound:
        std::cerr << "Error: Material not found: [" << result.data << "]\n";
        break;
      case ParseError::None:
        // Este caso ya se maneja arriba, pero se incluye por completitud.
        break;
    }

    std::cerr << "Line: \"" << original_line << "\"\n";
    return false;
  }

  // --- Lógica Central de Procesamiento (Ahora más corta) ---
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
      std::string_view entity_name = key;
      if (!key.empty() and key.back() == ':') {
        entity_name.remove_suffix(1);
      }
      std::cerr << "Error: Unknown scene entity: " << entity_name << "\n";
      return false;
    }

    ParseResult const result = it->second(tokens, scene);
    return logSceneParseResult(result, key, original_line);
  }

}  // namespace

// --- Interfaz Pública ---

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

  return scene;
}
