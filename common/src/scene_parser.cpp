#include "scene_parser.hpp"
#include "dataStructs/aabb.hpp"
#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
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
    // Combierte el string a tokens (más o menos cada palabra o dato importante)
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

  bool parsedouble(std::string_view token, double & value) {
    if (token.empty()) {
      return false;
    }

    char const * const start = token.data();
    char const * const end   = std::__to_address(token.end());

    auto const result = std::from_chars(start, end, value);

    return result.ec == std::errc() and result.ptr == end;
  }

  bool validateColorComponents(double r, double g, double b) {
    // Nos aseguramos de que los colores estan en los rangos correctos
    return r >= 0.0F and r <= 1.0F and g >= 0.0F and g <= 1.0F and b >= 0.0F and b <= 1.0F;
  }

  bool parseMatteMaterial(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() != 5) {
      std::cerr << "Error: matte material requires 4 parameters (name r g b), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    double r = 0.0F;
    double g = 0.0F;
    double b = 0.0F;

    if (!parsedouble(tokens[2], r) or !parsedouble(tokens[3], g) or !parsedouble(tokens[4], b)) {
      std::cerr << "Error: invalid color values for matte material\n";
      return false;
    }

    if (!validateColorComponents(r, g, b)) {
      std::cerr << "Error: color values must be in range [0, 1]\n";
      return false;
    }
    size_t const new_material_data_index = scene.matte.r.size();
    scene.matte.r.push_back(r);
    scene.matte.g.push_back(g);
    scene.matte.b.push_back(b);
    scene.materialTable.push_back(
        {MaterialType::MATTE, static_cast<unsigned int>(new_material_data_index)});
    scene.materialNames.emplace_back(tokens[1]);

    return true;
  }

  bool parseMetalMaterial(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() != 6) {
      std::cerr << "Error: metal material requires 5 parameters (name r g b "
                   "diffusion), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    double r         = 0.0F;
    double g         = 0.0F;
    double b         = 0.0F;
    double diffusion = 0.0F;

    if (!parsedouble(tokens[2], r) or
        !parsedouble(tokens[3], g) or
        !parsedouble(tokens[4], b) or
        !parsedouble(tokens[5], diffusion))
    {
      std::cerr << "Error: invalid parameter values for metal material\n";
      return false;
    }

    if (!validateColorComponents(r, g, b) or diffusion < 0.0F) {
      std::cerr << "Error: invalid parameter ranges for metal material\n";
      return false;
    }
    size_t const new_material_data_index = scene.metal.r.size();
    scene.metal.r.push_back(r);
    scene.metal.g.push_back(g);
    scene.metal.b.push_back(b);
    scene.metal.diffusion.push_back(diffusion);
    scene.materialTable.push_back(
        {MaterialType::METAL, static_cast<unsigned int>(new_material_data_index)});
    scene.materialNames.emplace_back(tokens[1]);

    return true;
  }

  bool parseRefractiveMaterial(std::vector<std::string_view> const & tokens,
                               SceneSettings & scene) {
    if (tokens.size() != 3) {
      std::cerr << "Error: refractive material requires 2 parameters (name ior), got "
                << tokens.size() - 1 << "\n";
      return false;
    }

    double ior = 0.0F;
    if (!parsedouble(tokens[2], ior)) {
      std::cerr << "Error: invalid IOR value for refractive material\n";
      return false;
    }

    if (ior <= 0.0F) {
      std::cerr << "Error: IOR must be positive\n";
      return false;
    }
    size_t const new_material_data_index = scene.refractive.ior.size();
    scene.refractive.ior.push_back(ior);
    scene.materialTable.push_back(
        {MaterialType::REFRACTIVE, static_cast<unsigned int>(new_material_data_index)});
    scene.materialNames.emplace_back(tokens[1]);

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

  bool parseSphere(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() != 6) {
      std::cerr << "Error: esfera requiere 5 parameteros (x y z radio material), "
                   "obtuvo "
                << tokens.size() - 1 << "\n";
      return false;
    }
    double x      = 0.0F;
    double y      = 0.0F;
    double z      = 0.0F;
    double radius = 0.0F;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], radius))
    {
      std::cerr << "Error: parametros de esfera incorrectos\n";
      return false;
    }
    if (radius <= 0.0F) {
      std::cerr << "Error: radio de la esfera debe ser positivo\n";
      return false;
    }
    int const materialIndex = findMaterialIndex(tokens[5], scene);
    if (materialIndex == -1) {
      std::cerr << "Error:material desconocido '" << tokens[5] << "' para esfera\n";
      return false;
    }

    scene.spheres.x.push_back(x);
    scene.spheres.y.push_back(y);
    scene.spheres.z.push_back(z);
    scene.spheres.r.push_back(radius);
    scene.spheres.materialIndex.push_back(static_cast<unsigned int>(materialIndex));
    scene.spheres.aabbs.push_back(AABB::from_sphere({x, y, z}, radius));  // generamos la caja AABB

    return true;
  }

  bool parseCylinder(std::vector<std::string_view> const & tokens, SceneSettings & scene) {
    if (tokens.size() != 9) {
      std::cerr << "Error: cilindro requiere 8 parámetros (x y z radio vx vy vz "
                   "material), obtuvo "
                << tokens.size() - 1 << "\n";
      return false;
    }
    double x = 0.0, y = 0.0, z = 0.0, vx = 0.0, vy = 0.0, vz = 0.0, radius = 0.0;
    if (!parsedouble(tokens[1], x) or
        !parsedouble(tokens[2], y) or
        !parsedouble(tokens[3], z) or
        !parsedouble(tokens[4], radius) or
        !parsedouble(tokens[5], vx) or
        !parsedouble(tokens[6], vy) or
        !parsedouble(tokens[7], vz))
    {
      std::cerr << "Error: parámetros del cilindro incorrectos\n";
      return false;
    }
    if (radius <= 0.0) {
      std::cerr << "Error: radio del cilindro debe ser positivo\n";
      return false;
    }
    if (vx == 0.0 and vy == 0.0 and vz == 0.0) {  // Check for zero axis vector
      std::cerr << "Error: vector de axis del cilindro no puede ser cero\n";
      return false;
    }
    int const materialIndex = findMaterialIndex(tokens[8], scene);
    if (materialIndex == -1) {
      std::cerr << "Error: material desconocido '" << tokens[8] << "' para cilindro\n";
      return false;
    }
    double axisLength = std::sqrt(vx * vx + vy * vy + vz * vz);
    scene.cylinders.addCentre(x, y, z);
    scene.cylinders.addAxis(vx, vy, vz);  // This will compute invAxisLen internally
    scene.cylinders.r.push_back(radius);
    scene.cylinders.materialIndex.push_back(materialIndex);
    scene.cylinders.addAABB(AABB::from_cylinder({x, y, z}, {vx, vy, vz}, radius,
                                                axisLength));  // Generamos la caja AABB
    return true;
  }

  bool processLine(std::string_view line, SceneSettings & scene) {
    trimWhitespace(line);

    if (line.empty() or line.front() == '#') {
      return true;
    }

    std::vector<std::string_view> const tokens = tokenizeLine(line);
    if (tokens.empty()) {
      return true;
    }

    std::string_view const command = tokens[0];

    if (command == "matte:") {
      return parseMatteMaterial(tokens, scene);
    }
    if (command == "metal:") {
      return parseMetalMaterial(tokens, scene);
    }
    if (command == "refractive:") {
      return parseRefractiveMaterial(tokens, scene);
    }
    if (command == "sphere:") {
      return parseSphere(tokens, scene);
    }
    if (command == "cylinder:") {
      return parseCylinder(tokens, scene);
    }

    std::cerr << "Error: unknown command '" << command << "'\n";
    return false;
  }

}  // namespace

[[nodiscard]] SceneSettings loadSceneFromFile(std::string const & filename) {
  SceneSettings scene;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open file '" << filename << "'\n";
    return scene;
  }

  std::string line;
  std::size_t lineNumber = 0;

  while (std::getline(file, line)) {
    ++lineNumber;
    if (!processLine(line, scene)) {
      std::cerr << "Error parsing line " << lineNumber << ": " << line << "\n";
    }
  }

  return scene;
}
