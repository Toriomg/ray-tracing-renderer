// FUNCIONES PARA EL PROCESAMIENTO DE LOS COLORES COMUNES A AMBAS RESPRESENTACIONES (AOS Y SOA)
#ifndef COLOR_UTILS_HPP
#define COLOR_UTILS_HPP

#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <cstdint>

namespace color_utils {

  // Aplica corrección gamma a un valor entre 0.0 y 1.0 para obtener colores válidos
  inline double apply_gamma(double value, double gamma) {
    const double clamped_value = std::clamp(value, 0.0, 1.0);
    return std::pow(clamped_value, 1.0 / gamma);
  }

  // Convierte un valor double [0,1] a uint8_t [0,255] para cumplir con los valores esperados de
  // color
  inline uint8_t double_to_uint8(double value) {
    const double clamped_value = std::clamp(value, 0.0, 1.0);
    return static_cast<uint8_t>(clamped_value * 255.999);
  }

}  // namespace color_utils

#endif
