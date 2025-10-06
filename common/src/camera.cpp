#include "camera.hpp"
#include "constants.hpp"

Camera::Camera(std::shared_ptr<ConfigSettings> config){
    cameraPos       = config->camera_pos;
    cameraTarget    = config->camera_target;
    cameraNorth     = config->camera_north;
    FOV             = config->field_of_view;

    focalVector = cameraPos - cameraTarget;

    float FOV_radians = FOV * (Constants::PI / 180.0f);
    ProjWindow.projWindowHeight = 2 * tan(FOV_radians/2.0f) * focalVector.length();

    std::pair AspRt = config->aspect_ratio;
    ProjWindow.projWindowWidth  = ProjWindow.projWindowHeight * AspRt.first / AspRt.second;
    // Vectores directores de la ventana
    Vec3 focalVectorNorm = focalVector.normalize();
    cameraRight = cross(cameraNorth, focalVectorNorm).normalize();
    cameraUp = cross(focalVectorNorm, cameraRight);

    // Vectores de la ventana de proyección
    ProjWindow.viewportHorizontal = ProjWindow.projWindowWidth * cameraRight;
    ProjWindow.viewportVertical   = ProjWindow.projWindowHeight * -cameraUp;

    ProjWindow.viewportOrigin = cameraTarget - 0.5f * 
        (ProjWindow.viewportHorizontal + ProjWindow.viewportVertical);
}