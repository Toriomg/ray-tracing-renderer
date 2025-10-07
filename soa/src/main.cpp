#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/ray.hpp"
#include "../../common/include/camera.hpp"
#include "../../common/include/constants.hpp"
#include "../../common/include/ppm_writer.hpp"

#include "color_utils.hpp"
#include "image_soa.hpp"
#include <iostream>
#include <string>

const std::string FilepathOut = "/workspace/outputImage.ppm";

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
            {0.0F, 5.0F, -5.0F},        // Centro en X
            {0.0F, 2.0F, -5.0F},        // Centro en Y
            {2.0F, 2.0F, 1.0F},       // Centro en Z
            {7.0F, 5.0F, 3.0F},        // Radio
            {0, 1, 2}            // Usa el material con ID 0
        }, // .spheres
        {},// .cylinders
        {
            { MaterialType::MATTE, 0 },
            { MaterialType::MATTE, 1 },
            { MaterialType::MATTE, 2 },
        }, // .materialTable
        {
            {0.8F, 0.5F, 0.0F},        // Componente Rojo
            {0.2F, 0.5F, 1.0F},        // Componente Verde
            {0.1F, 0.5F, 0.0F}         // Componente Azul
        }, // .materialMatte
        {},// .materialMetal
        {},// .materialRefractive
    });              
    auto camera = Camera(config);
    std::cout << "Generated camera\n";

    // Image width
    int imageWidth = camera.ProjWindow.imageWidth;
    int imageHeight = camera.ProjWindow.imageHeight;


    auto pixel_width = camera.ProjWindow.viewportHorizontal * (1.0F / static_cast<float>(imageWidth));
    auto pixel_height = camera.ProjWindow.viewportVertical * (1.0F / static_cast<float>(imageHeight));
    
    std::cout << "image size : " << imageWidth << " , " << imageHeight << "\n";
    ImageSOA image = ImageSOA(static_cast<size_t>(imageWidth), static_cast<size_t>(imageHeight));

    for(size_t row = 0; row < static_cast<size_t>(imageHeight); row++){
        for(size_t col = 0; col < static_cast<size_t>(imageWidth); col++){
            //Ray position in proj screen
            Point3 pixel_sample_point = camera.ProjWindow.viewportOrigin +
            pixel_width * static_cast<float>(col) +     // El ancho se multiplica por la columna
            pixel_height * static_cast<float>(row);
            
            // Crear el rayo
            Ray ray(camera.cameraPos, pixel_sample_point - camera.cameraPos);
            
            // Sacar el color del rayo
            Color pixel = rayColor(ray, *scene, *config);
            
            //TODO: esto es una cutrada pero es la forma rapida de settear el color de uno en uno sin crear unos buffers
            uint8_t red     = color_utils::float_to_uint8(pixel.x);
            uint8_t green   = color_utils::float_to_uint8(pixel.y);
            uint8_t blue    = color_utils::float_to_uint8(pixel.z);

            //Image save
            image.set_pixel(row, col, red, green, blue);

        }
    }
    if( !image.write_to_ppm(FilepathOut)){
        std::cerr << "Error writing into .ppm file";
    }
}
