#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/ray.hpp"
#include "../../common/include/camera.hpp"
#include "../../common/include/constants.hpp"
#include "image.hpp"
#include <iostream>

int main() {// NOLINT
    // Placeholders temporales
    // TODO: cambiar por los parsers
    std::shared_ptr<ConfigSettings> config = std::make_shared<ConfigSettings>(ConfigSettings{
        Constants::CameraPosition,      // .camera_pos
        Constants::CameraTarget,        // .camera_target
        Constants::CameraNorth,         // .camera_north
        Constants::FOV,                 // .field_of_view
        Constants::AspectRatio,         // .aspect_ratio
        900,          // .image_width
        Constants::Gamma,               // .gamma
        Constants::MaxDepth,            // .max_depth
        Constants::SamplesPerPixel,     // .samples_per_pixel
        Constants::RNGSeedMaterial,     // .material_rng_seed
        Constants::RNGSeedRay,          // .ray_rng_seed
        Constants::ColorBackgroundDark, // .background_dark_color
        Constants::ColorBackGroundLight,// .background_light_color
    });
    std::shared_ptr<SceneSettings> scene = std::make_shared<SceneSettings>(SceneSettings{
        {
            {0.0F},        // Centro en X
            {0.0F},        // Centro en Y
            {-1.0F},       // Centro en Z
            {2.0F},        // Radio
            {0}            // Usa el material con ID 0
        }, // .spheres
        {},// .cylinders
        {
            { MaterialType::MATTE, 0 }
        }, // .materialTable
        {
            {0.8F},        // Componente Rojo
            {0.2F},        // Componente Verde
            {0.1F}         // Componente Azul
        }, // .materialMatte
        {},// .materialMetal
        {},// .materialRefractive
    });              
    auto camera = Camera(config);
    Image image;
    std::cout << "Generated camera\n";
    auto pixel_width = camera.ProjWindow.viewportHorizontal * (1.0F / static_cast<float>(camera.ProjWindow.imageWidth));
    auto pixel_height = camera.ProjWindow.viewportVertical * (1.0F / static_cast<float>(camera.ProjWindow.imageHeight));
    
    std::cout << "image size : " << camera.ProjWindow.imageWidth << " , " << camera.ProjWindow.imageHeight << "\n";
    for(int row = 0; row < camera.ProjWindow.imageHeight; row++){
        for(int col = 0; col < camera.ProjWindow.imageWidth; col++){
            //Ray draw
            Point3 pixel_sample_point = camera.ProjWindow.viewportOrigin +
            pixel_width * static_cast<float>(col) +     // El ancho se multiplica por la columna
            pixel_height * static_cast<float>(row);
            
            Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos);
            
            Color pixel = rayColor(ray, *scene, *config);
            std::cout << "Color for pixel: " << pixel.x << " , " << pixel.y << " , " << pixel.z << "\n";
            //Image save
        }
    }

}
