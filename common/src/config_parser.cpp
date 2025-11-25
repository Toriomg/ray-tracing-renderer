#include "config_parser.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/vec3.hpp"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
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

  enum class ParseError { None, InvalidValue, ExtraData };

  // Agrupa el resultado de un análisis: el código de error y datos adicionales
  // para los mensajes de error (p. ej. texto sobrante en una línea).
  struct ParseResult {
    ParseError error_code;
    std::string extra_data;

    // Constructor para errores simples (sin datos extra).
    explicit ParseResult(ParseError code) : error_code(code) { }

    // Constructor para errores que necesitan reportar datos adicionales.
    ParseResult(ParseError code, std::string data)
        : error_code(code), extra_data(std::move(data)) { }
  };

  // --- Utilidades de bajo nivel para manipular strings sin crear copias ---

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

  // Une los tokens sobrantes de una línea para mostrarlos en un mensaje de error.
  std::string join_tokens(std::vector<std::string_view> const & tokens, std::size_t start_index) {
    if (start_index >= tokens.size()) {
      return "";
    }
    return std::accumulate(
        std::next(tokens.begin(), static_cast<long>(start_index + 1)), tokens.end(),
        std::string(tokens[start_index]),
        [](std::string const & a, std::string_view b) { return a + " " + std::string(b); });
  }

  // --- Parsers de Tipos Primitivos ---

  // Convierte un string_view a double usando la función más eficiente y segura (std::from_chars).
  [[nodiscard]] bool parsedouble(std::string_view token, double & value) {
    if (token.empty()) {
      return false;
    }
    // Parche para librerías antiguas donde from_chars no soporta el signo '+'.
    if (token.front() == '+') {
      token.remove_prefix(1);
    }
    auto const [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
    if (ec != std::errc() or ptr != token.data() + token.size()) {
      return false;
    }
    // Rechaza explícitamente "nan" e "inf", que son parseados con éxito por la librería.
    if (std::isnan(value) or std::isinf(value)) {
      return false;
    }
    return true;
  }

  [[nodiscard]] bool parseInt(std::string_view token, int & value) {
    if (token.empty()) {
      return false;
    }
    auto const [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
    return ec == std::errc() and ptr == token.data() + token.size();
  }

  [[nodiscard]] bool parseUnsignedLong(std::string_view token, unsigned long & value) {
    if (token.empty()) {
      return false;
    }
    auto const [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
    return ec == std::errc() and ptr == token.data() + token.size();
  }

  [[nodiscard]] bool parseUnsignedInt(std::string_view token, unsigned int & value) {
    if (token.empty()) {
      return false;
    }
    auto const [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
    return ec == std::errc() and ptr == token.data() + token.size();
  }

  // --- Funciones de Parseo para cada Clave de Configuración ---
  // Cada función valida el número exacto de argumentos para su clave.

  ParseResult parseAspectRatio(std::vector<std::string_view> const & tokens,
                               ConfigSettings & config) {
    if (tokens.size() < 3) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 3) {
      return {ParseError::ExtraData, join_tokens(tokens, 3)};
    }
    unsigned int width = 0, height = 0;
    if (!parseUnsignedInt(tokens[1], width) or
        !parseUnsignedInt(tokens[2], height) or
        width == 0 or
        height == 0)
    {
      return ParseResult(ParseError::InvalidValue);
    }
    config.aspect_ratio = {width, height};
    return ParseResult(ParseError::None);
  }

  ParseResult parseImageWidth(std::vector<std::string_view> const & tokens,
                              ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    int width = 0;
    if (!parseInt(tokens[1], width) or width <= 0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.image_width = width;
    return ParseResult(ParseError::None);
  }

  ParseResult parseGamma(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    double gamma = 0.0;
    if (!parsedouble(tokens[1], gamma)) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.gamma = gamma;
    return ParseResult(ParseError::None);
  }

  // Helper reutilizable para analizar cualquier clave que represente un vector de 3D.
  ParseResult parseVec3(std::vector<std::string_view> const & tokens, Vec3 & vec) {
    if (tokens.size() < 4) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 4) {
      return {ParseError::ExtraData, join_tokens(tokens, 4)};
    }
    double x = 0.0, y = 0.0, z = 0.0;
    if (!parsedouble(tokens[1], x) or !parsedouble(tokens[2], y) or !parsedouble(tokens[3], z)) {
      return ParseResult(ParseError::InvalidValue);
    }
    vec = Vec3(x, y, z);
    return ParseResult(ParseError::None);
  }

  // Helper reutilizable para analizar un color, validando que sus componentes estén en [0, 1].
  ParseResult parseColor(std::vector<std::string_view> const & tokens, Color & color) {
    if (tokens.size() < 4) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 4) {
      return {ParseError::ExtraData, join_tokens(tokens, 4)};
    }
    double r = 0.0, g = 0.0, b = 0.0;
    if (!parsedouble(tokens[1], r) or
        !parsedouble(tokens[2], g) or
        !parsedouble(tokens[3], b) or
        r < 0.0 or
        r > 1.0 or
        g < 0.0 or
        g > 1.0 or
        b < 0.0 or
        b > 1.0)
    {
      return ParseResult(ParseError::InvalidValue);
    }
    color = Color(r, g, b);
    return ParseResult(ParseError::None);
  }

  ParseResult parseCameraPosition(std::vector<std::string_view> const & tokens,
                                  ConfigSettings & config) {
    return parseVec3(tokens, config.camera_pos);
  }

  ParseResult parseCameraTarget(std::vector<std::string_view> const & tokens,
                                ConfigSettings & config) {
    return parseVec3(tokens, config.camera_target);
  }

  ParseResult parseCameraNorth(std::vector<std::string_view> const & tokens,
                               ConfigSettings & config) {
    return parseVec3(tokens, config.camera_north);
  }

  ParseResult parseFieldOfView(std::vector<std::string_view> const & tokens,
                               ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    double fov = 0.0;
    if (!parsedouble(tokens[1], fov) or fov <= 0.0 or fov >= 180.0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.field_of_view = fov;
    return ParseResult(ParseError::None);
  }

  ParseResult parseSamplesPerPixel(std::vector<std::string_view> const & tokens,
                                   ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    int samples = 0;
    if (!parseInt(tokens[1], samples) or samples <= 0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.samples_per_pixel = samples;
    return ParseResult(ParseError::None);
  }

  ParseResult parseMaxDepth(std::vector<std::string_view> const & tokens, ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    int depth = 0;
    if (!parseInt(tokens[1], depth) or depth <= 0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.max_depth = depth;
    return ParseResult(ParseError::None);
  }

  ParseResult parseMaterialRngSeed(std::vector<std::string_view> const & tokens,
                                   ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    unsigned long seed = 0;
    if (!parseUnsignedLong(tokens[1], seed) or seed <= 0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.material_rng_seed = seed;
    return ParseResult(ParseError::None);
  }

  ParseResult parseRayRngSeed(std::vector<std::string_view> const & tokens,
                              ConfigSettings & config) {
    if (tokens.size() < 2) {
      return ParseResult(ParseError::InvalidValue);
    }
    if (tokens.size() > 2) {
      return {ParseError::ExtraData, join_tokens(tokens, 2)};
    }
    unsigned long seed = 0;
    if (!parseUnsignedLong(tokens[1], seed) or seed <= 0) {
      return ParseResult(ParseError::InvalidValue);
    }
    config.ray_rng_seed = seed;
    return ParseResult(ParseError::None);
  }

  ParseResult parseBackgroundDarkColor(std::vector<std::string_view> const & tokens,
                                       ConfigSettings & config) {
    return parseColor(tokens, config.background_dark_color);
  }

  ParseResult parseBackgroundLightColor(std::vector<std::string_view> const & tokens,
                                        ConfigSettings & config) {
    return parseColor(tokens, config.background_light_color);
  }

  // Tabla de despacho: asocia cada clave de configuración con su función de parseo.
  // Este diseño hace muy fácil añadir nuevas claves sin modificar la lógica principal.
  using ParserFunc =
      std::function<ParseResult(std::vector<std::string_view> const &, ConfigSettings &)>;
  std::unordered_map<std::string_view, ParserFunc> const COMMANDS = {
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

  // Centraliza la lógica de impresión de errores.
  bool logParseResult(ParseResult const & result, std::string_view key,
                      std::string_view original_line) {
    switch (result.error_code) {
      case ParseError::None: return true;

      case ParseError::InvalidValue:
        std::cerr << "Error: Invalid value for key: [" << key << "]\n"
                  << "Line: \"" << original_line << "\"\n";
        return false;

      case ParseError::ExtraData:
        std::cerr << "Error: Extra data after configuration value for key: [" << key << "]\n"
                  << "Extra: \"" << result.extra_data << "\"\n";
        return false;
    }
    return false;
  }

  // Orquesta el análisis de una sola línea.
  bool processLine(std::string_view original_line, ConfigSettings & config) {
    auto line = original_line;
    trimWhitespace(line);
    if (line.empty() or line.front() == '#') {
      return true;  // Ignora líneas vacías y comentarios.
    }

    std::vector<std::string_view> const tokens = tokenizeLine(line);
    if (tokens.empty()) {
      return true;
    }

    auto const & key = tokens[0];
    auto it          = COMMANDS.find(key);

    if (it == COMMANDS.end()) {
      std::cerr << "Error: Unknown configuration key: [" << key << "]\n";
      return false;
    }

    // Delega el análisis a la función correspondiente y reporta el resultado.
    ParseResult const result = it->second(tokens, config);
    return logParseResult(result, key, original_line);
  }

}  // namespace

// --- Interfaz Pública ---

std::optional<ConfigSettings> loadConfigFromFile(std::string const & filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open config file '" << filename << "'\n";
    return std::nullopt;
  }

  // Se inicializa la configuración con valores por defecto.
  // Estos valores se mantendrán si no se especifican en el archivo.
  ConfigSettings config;
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

  std::string line;
  std::size_t lineNumber = 0;
  while (std::getline(file, line)) {
    ++lineNumber;
    if (!processLine(line, config)) {
      // Si cualquier línea falla, se añade el contexto del número de línea y se aborta.
      std::cerr << "-> Error occurred while parsing line " << lineNumber << "\n";
      return std::nullopt;
    }
  }

  return config;
}
