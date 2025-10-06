#pragma once
#include "vec3.hpp"
#include "dataStructs/settings_structs.hpp"

struct ProjectionWindow {
    float projWindowHeight;
    float projWindowWidth;
    float imageHeight;
    float imageWidth;
    Vec3 viewportHorizontal;
    Vec3 viewportVertical;
    Point3 viewportOrigin;
};

class Camera{
    public:
    Vec3 cameraPos;
    Point3 cameraTarget;
    Vec3 cameraNorth;
    float FOV;
    ProjectionWindow ProjWindow;
    Vec3 cameraRight;
    Vec3 cameraUp;
    Vec3 focalVector;
    
    Camera(std::shared_ptr<ConfigSettings> config);
};