#include "camera.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <gtest/gtest.h>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA Camera
// ============================================================================

class CameraTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza común si es necesaria
  }
};

// ============================================================================
// TESTS PARA Camera::Camera (Constructor)
// ============================================================================

// Test 1: Configuración simple válida
TEST_F(CameraTest, ValidSimpleSetup) {
  // Configuración: cámara en el origen, mirando a -Z, FOV 90, aspect 1:1
  ConfigSettings config;

  config.camera_pos    = Point3(0, 0, 0);
  config.camera_target = Point3(0, 0, -1);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 90.0;
  config.aspect_ratio  = {1, 1};
  config.image_width   = 100;

  // Llamar al constructor
  Camera const camera(config);

  // Verificar los resultados calculados

  // focalVector = pos - target = (0,0,0) - (0,0,-1) = (0,0,1)
  ASSERT_DOUBLE_EQ(camera.focalVector.x, 0.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.y, 0.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.z, 1.0);

  // projWindowHeight = 2 * tan(FOV_rad/2) * focalLength
  // FOV_rad = 90 * (PI/180) = PI/2
  // tan(PI/4) = 1.0
  // focalLength = length(focalVector) = 1.0
  // projWindowHeight = 2 * 1.0 * 1.0 = 2.0
  ASSERT_DOUBLE_EQ(camera.ProjWindow.projWindowHeight, 2.0);

  // projWindowWidth = projWindowHeight * (aspect_w / aspect_h) = 2.0 * (1/1) = 2.0
  ASSERT_DOUBLE_EQ(camera.ProjWindow.projWindowWidth, 2.0);

  // cameraRight = cross(north, focalNorm) = cross((0,1,0), (0,0,1)) = (1,0,0)
  ASSERT_DOUBLE_EQ(camera.cameraRight.x, 1.0);
  ASSERT_DOUBLE_EQ(camera.cameraRight.y, 0.0);
  ASSERT_DOUBLE_EQ(camera.cameraRight.z, 0.0);

  // cameraUp = cross(focalNorm, cameraRight) = cross((0,0,1), (1,0,0)) = (0,1,0)
  ASSERT_DOUBLE_EQ(camera.cameraUp.x, 0.0);
  ASSERT_DOUBLE_EQ(camera.cameraUp.y, 1.0);
  ASSERT_DOUBLE_EQ(camera.cameraUp.z, 0.0);

  // viewportHorizontal = projWindowWidth * cameraRight = 2.0 * (1,0,0) = (2,0,0)
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportHorizontal.x, 2.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportHorizontal.y, 0.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportHorizontal.z, 0.0);

  // viewportVertical = projWindowHeight * -cameraUp = 2.0 * -(0,1,0) = (0,-2,0)
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportVertical.x, 0.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportVertical.y, -2.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportVertical.z, 0.0);

  // imageHeight = imageWidth * (aspect_h / aspect_w) = 100 * (1/1) = 100
  ASSERT_EQ(camera.ProjWindow.imageHeight, 100);

  // viewportOrigin = target - 0.5 * (horizontal + vertical)
  // = (0,0,-1) - 0.5 * ((2,0,0) + (0,-2,0))
  // = (0,0,-1) - 0.5 * (2,-2,0)
  // = (0,0,-1) - (1,-1,0)
  // = (-1, 1, -1)
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportOrigin.x, -1.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportOrigin.y, 1.0);
  ASSERT_DOUBLE_EQ(camera.ProjWindow.viewportOrigin.z, -1.0);
}

// Test 2: Configuración con cámara desplazada y aspect ratio 16:9
TEST_F(CameraTest, ValidOffsetSetup) {
  // Configuración: cámara desplazada, aspect 16:9
  ConfigSettings config;

  config.camera_pos    = Point3(0, 2, 5);
  config.camera_target = Point3(0, 0, 0);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 60.0;
  config.aspect_ratio  = {16, 9};
  config.image_width   = 160;

  // Llamar al constructor
  Camera const camera(config);

  // Verificar resultados clave

  // focalVector = (0,2,5) - (0,0,0) = (0,2,5)
  ASSERT_DOUBLE_EQ(camera.focalVector.x, 0.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.y, 2.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.z, 5.0);

  // focalVector normalizado = (0, 2/sqrt(29), 5/sqrt(29))
  // cross(north, focalNorm) = cross((0,1,0), (0, 2/sqrt(29), 5/sqrt(29)))
  // = (5/sqrt(29), 0, 0) normalizado = (1, 0, 0) o (-1, 0, 0)
  // Debe ser (1,0,0) ya que cross product sigue regla de mano derecha
  ASSERT_NEAR(camera.cameraRight.x, 1.0, 1e-9);
  ASSERT_NEAR(camera.cameraRight.y, 0.0, 1e-9);
  ASSERT_NEAR(camera.cameraRight.z, 0.0, 1e-9);

  // cameraUp = cross(focalNorm, cameraRight)
  // Debe ser perpendicular a ambos y apuntar "hacia arriba"
  // cross((0, 2/sqrt(29), 5/sqrt(29)), (1,0,0)) = (0, -5/sqrt(29), 2/sqrt(29))
  ASSERT_NEAR(camera.cameraUp.x, 0.0, 1e-9);
  ASSERT_NEAR(camera.cameraUp.y, 5.0 / std::sqrt(29.0), 1e-9);
  ASSERT_NEAR(camera.cameraUp.z, -2.0 / std::sqrt(29.0), 1e-9);

  // imageHeight = imageWidth * (aspect_h / aspect_w) = 160 * (9.0/16.0) = 90
  ASSERT_EQ(camera.ProjWindow.imageHeight, 90);

  // Verificar relación de aspecto en ventana de proyección
  // projWindowWidth = projWindowHeight * (16.0/9.0)
  ASSERT_NEAR(camera.ProjWindow.projWindowWidth, camera.ProjWindow.projWindowHeight * (16.0 / 9.0),
              1e-9);
}

// Test 3: Caso borde - misma posición y target (genera NaN)
TEST_F(CameraTest, EdgeCaseSamePositionAndTarget) {
  // Configuración: pos == target (bug: focalVector = 0, normalize(0) = NaN)
  ConfigSettings config;

  config.camera_pos    = Point3(1, 1, 1);
  config.camera_target = Point3(1, 1, 1);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 90.0;
  config.aspect_ratio  = {1, 1};
  config.image_width   = 100;

  // Llamar al constructor
  Camera const camera(config);

  // Verificar el resultado: focalVector = (0,0,0), normalize(0) produce NaN

  // focalVector debería ser (0,0,0)
  ASSERT_DOUBLE_EQ(camera.focalVector.x, 0.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.y, 0.0);
  ASSERT_DOUBLE_EQ(camera.focalVector.z, 0.0);
}

// Test 4: FOV muy pequeño (caso borde)
TEST_F(CameraTest, SmallFOV) {
  // Configuración: FOV muy pequeño (1 grado)
  ConfigSettings config;

  config.camera_pos    = Point3(0, 0, 10);
  config.camera_target = Point3(0, 0, 0);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 1.0;
  config.aspect_ratio  = {1, 1};
  config.image_width   = 100;

  // Llamar al constructor
  Camera const camera(config);

  // Con FOV muy pequeño, la ventana de proyección debe ser muy pequeña
  // FOV_rad = 1.0 * (PI/180) ≈ 0.0174533
  // tan(FOV_rad/2) ≈ tan(0.008727) ≈ 0.008727
  // focalLength = 10.0
  // projWindowHeight = 2 * 0.008727 * 10.0 ≈ 0.17454

  double const expected_height = 2.0 * std::tan((1.0 * Constants::PI / 180.0) / 2.0) * 10.0;
  ASSERT_NEAR(camera.ProjWindow.projWindowHeight, expected_height, 1e-9);

  // projWindowWidth debe ser igual (aspect 1:1)
  ASSERT_NEAR(camera.ProjWindow.projWindowWidth, expected_height, 1e-9);
}

// Test 5: FOV muy grande (caso borde)
TEST_F(CameraTest, LargeFOV) {
  // Configuración: FOV muy grande (170 grados)
  ConfigSettings config;

  config.camera_pos    = Point3(0, 0, 1);
  config.camera_target = Point3(0, 0, 0);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 170.0;
  config.aspect_ratio  = {1, 1};
  config.image_width   = 100;

  // Llamar al constructor
  Camera const camera(config);

  // Con FOV muy grande, la ventana de proyección debe ser muy grande
  // FOV_rad = 170 * (PI/180) ≈ 2.9671
  // tan(FOV_rad/2) ≈ tan(1.4835) ≈ 11.43
  // focalLength = 1.0
  // projWindowHeight = 2 * 11.43 * 1.0 ≈ 22.86

  double const expected_height = 2.0 * std::tan((170.0 * Constants::PI / 180.0) / 2.0) * 1.0;
  ASSERT_NEAR(camera.ProjWindow.projWindowHeight, expected_height, 1e-6);

  // projWindowWidth debe ser igual (aspect 1:1)
  ASSERT_NEAR(camera.ProjWindow.projWindowWidth, expected_height, 1e-6);
}

// Test 6: Aspect ratio extremo (muy ancho)
TEST_F(CameraTest, ExtremeAspectRatio) {
  // Configuración: aspect ratio muy ancho (32:9)
  ConfigSettings config;

  config.camera_pos    = Point3(0, 0, 5);
  config.camera_target = Point3(0, 0, 0);
  config.camera_north  = Vec3(0, 1, 0);
  config.field_of_view = 90.0;
  config.aspect_ratio  = {32, 9};
  config.image_width   = 320;

  // Llamar al constructor
  Camera const camera(config);

  // imageHeight = 320 * (9.0/32.0) = 90
  ASSERT_EQ(camera.ProjWindow.imageHeight, 90);

  // projWindowWidth debe ser mucho mayor que projWindowHeight
  // projWindowWidth = projWindowHeight * (32.0/9.0)
  ASSERT_NEAR(camera.ProjWindow.projWindowWidth, camera.ProjWindow.projWindowHeight * (32.0 / 9.0),
              1e-9);
}

// Test 7: Cámara con north no ortogonal a focal (caso común)
TEST_F(CameraTest, NonOrthogonalNorth) {
  // Configuración: north no perfectamente ortogonal a focalVector
  ConfigSettings config;

  config.camera_pos    = Point3(5, 5, 5);
  config.camera_target = Point3(0, 0, 0);
  config.camera_north  = Vec3(0, 1, 0);  // north está en Y, pero focal no está en plano XZ
  config.field_of_view = 60.0;
  config.aspect_ratio  = {16, 9};
  config.image_width   = 160;

  // Llamar al constructor
  Camera const camera(config);

  // Verificar que cameraRight y cameraUp son ortogonales entre sí
  double const dot_right_up = dot(camera.cameraRight, camera.cameraUp);
  ASSERT_NEAR(dot_right_up, 0.0, 1e-9);

  // Verificar que cameraRight es ortogonal a focalVector normalizado
  Vec3 const focal_norm        = camera.focalVector.normalize();
  double const dot_right_focal = dot(camera.cameraRight, focal_norm);
  ASSERT_NEAR(dot_right_focal, 0.0, 1e-9);

  // Verificar que cameraUp es ortogonal a focalVector normalizado
  double const dot_up_focal = dot(camera.cameraUp, focal_norm);
  ASSERT_NEAR(dot_up_focal, 0.0, 1e-9);

  // Verificar que cameraRight y cameraUp tienen longitud 1 (son normalizados)
  ASSERT_NEAR(camera.cameraRight.length(), 1.0, 1e-9);
  ASSERT_NEAR(camera.cameraUp.length(), 1.0, 1e-9);
}
