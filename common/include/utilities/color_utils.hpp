// FUNCIONES PARA EL PROCESAMIENTO DE LOS COLORES COMUNES A AMBAS RESPRESENTACIONES (AOS Y SOA)
#ifndef COLOR_UTILS_HPP
#define COLOR_UTILS_HPP

#include <cmath>
#include <cstdint>

namespace color_utils {

  // Aplica corrección gamma a un valor entre 0.0 y 1.0 para obtener colores válidos
  inline float apply_gamma(float value, float gamma) {
    if (value <= 0.0F) {
      return 0.0F;
    }
    if (value >= 1.0F) {
      return 1.0F;
    }
    return std::pow(value, 1.0F / gamma);
  }

  // Convierte un valor float [0,1] a uint8_t [0,255] para cumplir con los valores esperados de
  // color
  inline uint8_t float_to_uint8(float value) {
    if (value <= 0.0F) {
      return 0;
    }
    if (value >= 1.0F) {
      return 255;
    }
    return static_cast<uint8_t>(value * 255.0F);
  }

}  // namespace color_utils

#endif
