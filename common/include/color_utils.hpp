// FUNCIONES PARA EL PROCESAMIENTO DE LOS COLORES COMUNES A AMBAS RESPRESENTACIONES (AOS Y SOA)
#pragma once

#include <cstdint>
#include <cmath>

namespace color_utils {
    
    // Aplica corrección gamma a un valor entre 0.0 y 1.0
    inline float apply_gamma(float value, float gamma) {
        if (value <= 0.0f) return 0.0f;
        if (value >= 1.0f) return 1.0f;
        return std::pow(value, 1.0f / gamma);
    }
    
    // Convierte un valor float [0,1] a uint8_t [0,255]
    inline uint8_t float_to_uint8(float value) {
        if (value <= 0.0f) return 0;
        if (value >= 1.0f) return 255;
        return static_cast<uint8_t>(value * 255.0f);
    }
    
} 