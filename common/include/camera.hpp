#pragma once
#include "utilities/vec3.hpp"
#include <memory>

struct ConfigSettings;


struct ProjectionWindow {
    // Initialize members to default values to prevent garbage data.
    float projWindowHeight = 0.0F;
    float projWindowWidth = 0.0F;
    int imageHeight = 0;
    int imageWidth = 0;
    Vec3 viewportHorizontal;
    Vec3 viewportVertical;
    Point3 viewportOrigin;
};

class Camera{
    public:
    Vec3 cameraPos;
    Point3 cameraTarget;
    Vec3 cameraNorth;
    float FOV = 0.0F;
    ProjectionWindow ProjWindow;
    Vec3 cameraRight;
    Vec3 cameraUp;
    Vec3 focalVector;
    
    Camera(std::shared_ptr<ConfigSettings>& config);
};