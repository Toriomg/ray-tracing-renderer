#pragma once

#include "vec3.hpp"
#include <limits>

namespace Constants {

    // --- Constantes Matemáticas ---
    inline constexpr float PI = 3.1415926535897932385f;
    inline constexpr float Infinity = std::numeric_limits<float>::infinity();
    
    // --- Parámetros por Defecto de la Configuración ---
    // También puedes poner aquí los valores por defecto que se mencionan en el documento del proyecto.
    namespace Defaults {
        inline constexpr std::pair<int, int> AspectRatio = {16, 9};
        inline constexpr int    ImageWidth = 1920;

        inline constexpr Point3 CameraPosition(0.0f, 0.0f, -10.0f);
        inline constexpr Point3 CameraTarget(0.0f, 0.0f, 0.0f);
        inline constexpr Vec3   CameraNorth(0.0f, 1.0f, 0.0f);
        inline constexpr float  FOV = 90.0f;

        inline constexpr float  Gamma = 2.2f;
        inline constexpr int    SamplesPerPixel = 20;
        inline constexpr int    MaxDepth = 5;

        // TODO: select a correct integer type
        inline constexpr int    RNGSeedMaterial = 13;
        inline constexpr int    RNGSeedRay = 19;

        inline constexpr Color ColorBackGroundLight(1.0f, 1.0f, 1.0f);
        inline constexpr Color ColorBackgroundDark(0.25f, 0.5f, 1.0f);
    }

}