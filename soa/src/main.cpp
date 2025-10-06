#include "../../common/include/dataStructs/settings_structs.hpp"
#include "../../common/include/ray.hpp"
#include "../../common/include/camera.hpp"
#include "../../common/include/constants.hpp"
#include "image.hpp"

int main() {
    // Placeholders temporales
    // TODO: cambiar por los parsers
    std::shared_ptr<ConfigSettings> config = std::make_shared<ConfigSettings>(ConfigSettings{
        Constants::CameraPosition,      // .camera_pos
        Constants::CameraTarget,        // .camera_target
        Constants::CameraNorth,         // .camera_north
        Constants::FOV,                 // .field_of_view
        Constants::AspectRatio,         // .aspect_ratio
        Constants::ImageWidth,          // .image_width
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
            {0.0f},        // Centro en X
            {0.0f},        // Centro en Y
            {-1.0f},       // Centro en Z
            {0.5f},        // Radio
            {0}            // Usa el material con ID 0
        }, // .spheres
        {},// .cylinders
        {
            { MaterialType::MATTE, 0 }
        }, // .materialTable
        {
            {0.8f},        // Componente Rojo
            {0.2f},        // Componente Verde
            {0.1f}         // Componente Azul
        }, // .materialMatte
        {},// .materialMetal
        {},// .materialRefractive
    });              
    Camera camera = Camera(config);
    Image image;

    Vec3 pixel_width = camera.ProjWindow.viewportHorizontal / camera.ProjWindow.imageWidth;
    Vec3 pixel_height = camera.ProjWindow.viewportVertical / camera.ProjWindow.imageHeight;
    
    for(unsigned int row = 0; row < camera.ProjWindow.imageHeight; row++){
        for(unsigned int col = 0; col < camera.ProjWindow.imageWidth; col++){
            //Ray draw
            Point3 pixel_sample_point = camera.ProjWindow.viewportOrigin +
                pixel_width * (row) +
                pixel_height * (col);
            
            Vec3 Ray_dir = camera.cameraPos - pixel_sample_point;
            Ray ray(pixel_sample_point, Ray_dir);

            Color pixel = rayColor(ray, *scene, *config);
            //Image save
        }
    }

}
