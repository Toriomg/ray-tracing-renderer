#include "vec3.hpp"
#include "object.hpp"
#include "material.hpp"
#include <vector>

struct ConfigSttings{
    Point3 camera_pos;
    Point3 camera_target;
    Vec3 camera_north;
    float field_of_view;
    std::pair<unsigned int, unsigned int> aspect_ratio;
    int    image_width;
    float  gamma;
    int    max_depth;
    int    samples_per_pixel;
    int    material_rng_seed;
    int    ray_rng_seed;
    Color  background_dark_color;
    Color  background_light_color;
};

struct SceneSettings{
    SphereData spheres;
    CylinderData cylinders;
    std::vector<MaterialID> materialTable;
};