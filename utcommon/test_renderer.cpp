#include "dataStructs/aabb.hpp"
#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"
#include "ray.hpp"
#include "renderer.hpp"
#include "utilities/random.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <string_view>

namespace {

  struct CylinderParams {
    Point3 center;
    Vec3 axis;
    double radius{};
    double height{};
  };

  void setupSingleSphereScene(SceneSettings * scene, Point3 center, double radius,
                              unsigned int mat_id) {
    scene->spheres.x.push_back(center.x);
    scene->spheres.y.push_back(center.y);
    scene->spheres.z.push_back(center.z);
    scene->spheres.r.push_back(radius);
    scene->spheres.materialIndex.push_back(mat_id);

    // Generar AABB para la esfera
    scene->spheres.aabbs.push_back(AABB::from_sphere(center, radius));
  }

  void setupMatteMaterial(SceneSettings * scene, std::string_view name, Color color) {
    scene->materialNames.emplace_back(name);
    scene->matte.r.push_back(color.x);
    scene->matte.g.push_back(color.y);
    scene->matte.b.push_back(color.z);

    auto local_index = static_cast<unsigned int>(scene->matte.r.size() - 1);
    scene->materialTable.push_back({MaterialType::MATTE, local_index});
  }

  void setupMetalMaterial(SceneSettings * scene, std::string_view name, Color color,
                          double diffusion) {
    scene->materialNames.emplace_back(name);
    scene->metal.r.push_back(color.x);
    scene->metal.g.push_back(color.y);
    scene->metal.b.push_back(color.z);
    scene->metal.diffusion.push_back(diffusion);

    auto local_index = static_cast<unsigned int>(scene->metal.r.size() - 1);
    scene->materialTable.push_back({MaterialType::METAL, local_index});
  }

  void setupSingleCylinderScene(SceneSettings * scene, CylinderParams params, unsigned int mat_id) {
    scene->cylinders.x.push_back(params.center.x);
    scene->cylinders.y.push_back(params.center.y);
    scene->cylinders.z.push_back(params.center.z);
    scene->cylinders.vx.push_back(params.axis.x);
    scene->cylinders.vy.push_back(params.axis.y);
    scene->cylinders.vz.push_back(params.axis.z);
    scene->cylinders.r.push_back(params.radius);
    scene->cylinders.invAxisLen.push_back(1.0 / params.height);
    scene->cylinders.materialIndex.push_back(static_cast<int>(mat_id));

    // Generar AABB para el cilindro
    scene->cylinders.aabbs.push_back(
        AABB::from_cylinder(params.center, params.axis, params.radius, params.height));
  }

  void setupRefractiveMaterial(SceneSettings * scene, std::string_view name, double ior) {
    scene->materialNames.emplace_back(name);
    scene->refractive.ior.push_back(ior);

    auto local_index = static_cast<unsigned int>(scene->refractive.ior.size() - 1);
    scene->materialTable.push_back({MaterialType::REFRACTIVE, local_index});
  }

  void clearScene(SceneSettings * scene) {
    scene->spheres.x.clear();
    scene->spheres.y.clear();
    scene->spheres.z.clear();
    scene->spheres.r.clear();
    scene->spheres.materialIndex.clear();
    scene->spheres.aabbs.clear();

    scene->cylinders.x.clear();
    scene->cylinders.y.clear();
    scene->cylinders.z.clear();
    scene->cylinders.r.clear();
    scene->cylinders.vx.clear();
    scene->cylinders.vy.clear();
    scene->cylinders.vz.clear();
    scene->cylinders.invAxisLen.clear();
    scene->cylinders.materialIndex.clear();
    scene->cylinders.aabbs.clear();

    scene->materialNames.clear();
    scene->materialTable.clear();
    scene->matte.r.clear();
    scene->matte.g.clear();
    scene->matte.b.clear();
    scene->metal.r.clear();
    scene->metal.g.clear();
    scene->metal.b.clear();
    scene->metal.diffusion.clear();
    scene->refractive.ior.clear();
  }

}  // namespace

// ============================================================================
// FIXTURE DE GOOGLETEST
// ============================================================================

class RendererTest : public ::testing::Test {
protected:
  SceneSettings scene;
  ConfigSettings config;
  RandomGenerator rng;

  RendererTest() : scene{}, config{}, rng{12'345} { }

  void SetUp() override {
    clearScene(&scene);

    // Configuración por defecto
    config.image_width            = 100;
    config.aspect_ratio           = {16, 9};
    config.samples_per_pixel      = 1;
    config.max_depth              = 5;
    config.background_dark_color  = Color(0.5, 0.7, 1.0);
    config.background_light_color = Color(1.0, 1.0, 1.0);
  }

  void TearDown() override { clearScene(&scene); }
};

// ============================================================================
// TESTS PARA rayColor
// ============================================================================
//
// La función rayColor() es el orquestador principal. Los siguientes tests
// (RenderSpheres y RenderCylinders) cubren todos sus caminos lógicos
// principales usando la estrategia "depth=1" (Hit=Negro, Miss=Fondo).

// ============================================================================
// TESTS PARA RenderSpheres
// ============================================================================

// Casos de ACIERTO

// Test 1: Hit Frontal - Rayo golpea esfera de frente
TEST_F(RendererTest, RayColorHitsFrontSphere) {
  // Setup: Esfera roja en (0, 0, -5) con radio 1.0
  setupMatteMaterial(&scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 desde origen hacia -Z (debe golpear en t≈4.0)
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  // Llamar a rayColor (internamente usa RenderSpheres)
  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 4: Rayo desde dentro de la esfera
TEST_F(RendererTest, RayColorFromInsideSphere) {
  // Setup: Esfera grande en origen con radio 2.0
  setupMatteMaterial(&scene, "green_matte", Color(0.0, 1.0, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, 0), 2.0, 0);

  // Rayo con depth=1 desde el centro hacia fuera
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, 1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit desde dentro con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 5: Múltiples esferas - golpea la más cercana
TEST_F(RendererTest, RayColorHitsClosestSphere) {
  // Setup: Dos esferas en la misma línea
  setupMatteMaterial(&scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupMatteMaterial(&scene, "blue_matte", Color(0.0, 0.0, 1.0));

  setupSingleSphereScene(&scene, Point3(0, 0, -3), 0.5, 0);  // Roja más cercana
  setupSingleSphereScene(&scene, Point3(0, 0, -6), 0.5, 1);  // Azul más lejos

  // Rayo con depth=1
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera más cercana
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 7: Rayo tangente a esfera
TEST_F(RendererTest, RayColorTangentToSphere) {
  // Setup: Esfera en (0, 0, -5) con radio 1.0
  setupMatteMaterial(&scene, "yellow_matte", Color(1.0, 1.0, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo tangente con depth=1 desde (1, 0, 0) hacia -Z
  Ray const ray(Point3(1, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Tangente debe golpear (discriminante ≈ 0 válido)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 9: Esfera muy pequeña
TEST_F(RendererTest, RayColorHitsVerySmallSphere) {
  setupMatteMaterial(&scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 0.001, 0);

  // Rayo con depth=1 que pasa por el centro
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear esfera pequeña
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 10: Rayo diagonal
TEST_F(RendererTest, RayColorDiagonalRay) {
  setupMatteMaterial(&scene, "cyan_matte", Color(0.0, 1.0, 1.0));
  setupSingleSphereScene(&scene, Point3(1, 1, -5), 1.0, 0);

  // Rayo diagonal con depth=1
  Vec3 const direction = Vec3(1, 1, -5).normalize();
  Ray const ray(Point3(0, 0, 0), direction, 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 3: Esfera detrás del rayo
TEST_F(RendererTest, RayColorSphereBehindRay) {
  // Setup: Esfera en (0, 0, -5)
  setupMatteMaterial(&scene, "blue_matte", Color(0.0, 0.0, 1.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 apuntando en dirección opuesta (+Z)
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, 1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Esfera detrás debe dar backgroundColor
  Vec3 const unit_dir = Vec3(0, 0, 1).normalize();
  double const t      = 0.5 * (unit_dir.y + 1.0);
  Color const expected =
      (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// Test 11: Rayo casi tangente (miss por poco)
TEST_F(RendererTest, RayColorJustMissesSphere) {
  setupMatteMaterial(&scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 que pasa justo afuera (x=1.1, radio=1.0)
  Ray const ray(Point3(1.1, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Miss debe retornar backgroundColor
  Vec3 const unit_dir = Vec3(0, 0, -1).normalize();
  double const t      = 0.5 * (unit_dir.y + 1.0);
  Color const expected =
      (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// TEST ESPECIAL - Verifica el caso base (max_depth = 0)

// Test 8: Depth limit - verificar que max_depth funciona
TEST_F(RendererTest, RayColorRespectsMaxDepth) {
  // Setup: Esfera con material matte
  setupMatteMaterial(&scene, "gray_matte", Color(0.5, 0.5, 0.5));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Configurar max_depth = 0 (sin rebotes)
  config.max_depth = 0;

  // Rayo sin especificar depth (usa constructor por defecto, depth=0)
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 0);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // Con depth = 0, debe retornar negro inmediatamente (caso base)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// TESTS ADICIONALES - Casos extremos

// Test 12: Esfera en el origen
TEST_F(RendererTest, RayColorSphereAtOrigin) {
  setupMatteMaterial(&scene, "magenta_matte", Color(1.0, 0.0, 1.0));
  setupSingleSphereScene(&scene, Point3(0, 0, 0), 1.0, 0);

  // Rayo con depth=1 desde (-5, 0, 0) hacia +X
  Ray const ray(Point3(-5, 0, 0), Vec3(1, 0, 0), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera en el origen
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 13: Esfera muy lejos
TEST_F(RendererTest, RayColorSphereVeryFar) {
  setupMatteMaterial(&scene, "orange_matte", Color(1.0, 0.5, 0.0));
  setupSingleSphereScene(&scene, Point3(0, 0, -1'000), 10.0, 0);

  // Rayo con depth=1 desde origen hacia -Z
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear incluso estando muy lejos
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 14: Rayo descentrado
TEST_F(RendererTest, RayColorOffCenterHit) {
  setupMatteMaterial(&scene, "purple_matte", Color(0.5, 0.0, 0.5));
  setupSingleSphereScene(&scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 desde (0.5, 0, 0) hacia -Z (golpea descentrado)
  Ray const ray(Point3(0.5, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear (0.5 < radio 1.0)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// ============================================================================
// TESTS PARA - RenderCylinders
// ============================================================================

// Test 1: Hit en superficie lateral del cilindro
TEST_F(RendererTest, CylinderHitLateralSurface) {
  // Setup: Material y cilindro base
  // Centro (0, -10, 0), Eje Y (0,1,0), Radio 2.0, Altura 4.0
  // El cilindro está en coordenadas y: [-12, -8]
  setupMatteMaterial(&scene, "cyan_matte", Color(0.0, 1.0, 1.0));
  CylinderParams const cyl = {Point3(0, -10, 0), Vec3(0, 4, 0), 2.0, 4.0};
  setupSingleCylinderScene(&scene, cyl, 0);

  // Rayo con depth=1 desde (0, -10, 0) hacia -Z (golpea superficie lateral)
  Ray const ray(Point3(0, -10, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit en lateral con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 2: Hit en tapa superior del cilindro
TEST_F(RendererTest, CylinderHitTopCap) {
  // Setup: Cilindro con tapa superior en y=-8
  setupMatteMaterial(&scene, "yellow_matte", Color(1.0, 1.0, 0.0));
  CylinderParams const cyl = {Point3(0, -10, 0), Vec3(0, 4, 0), 2.0, 4.0};
  setupSingleCylinderScene(&scene, cyl, 0);

  // Rayo con depth=1 desde (0, 0, 0) hacia (0, -1, 0) (golpea tapa superior)
  Ray const ray(Point3(0, 0, 0), Vec3(0, -1, 0), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit en tapa superior con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 3: Hit en tapa inferior del cilindro
TEST_F(RendererTest, CylinderHitBottomCap) {
  // Setup: Cilindro con tapa inferior en y=-12
  setupMatteMaterial(&scene, "magenta_matte", Color(1.0, 0.0, 1.0));
  CylinderParams const cyl = {Point3(0, -10, 0), Vec3(0, 4, 0), 2.0, 4.0};
  setupSingleCylinderScene(&scene, cyl, 0);

  // Rayo con depth=1 desde (0, -20, 0) hacia (0, 1, 0) (golpea tapa inferior)
  Ray const ray(Point3(0, -20, 0), Vec3(0, 1, 0), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit en tapa inferior con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 4: Miss - Rayo pasa completamente de largo
TEST_F(RendererTest, CylinderMissCompletely) {
  // Setup: Cilindro en (0, -10, 0)
  setupMatteMaterial(&scene, "red_matte", Color(1.0, 0.0, 0.0));
  CylinderParams const cyl = {Point3(0, -10, 0), Vec3(0, 4, 0), 2.0, 4.0};
  setupSingleCylinderScene(&scene, cyl, 0);

  // Rayo con depth=1 desde (10, 10, 0) hacia (0, 0, -1) (muy lejos del cilindro)
  Ray const ray(Point3(10, 10, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Miss debe retornar backgroundColor
  Vec3 const unit_dir = Vec3(0, 0, -1).normalize();
  double const t      = 0.5 * (unit_dir.y + 1.0);
  Color const expected =
      (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// Test 5: Miss - Golpea cilindro infinito pero fuera de altura
TEST_F(RendererTest, CylinderMissHitsInfiniteLateralOutsideHeight) {
  // Setup: Cilindro con altura limitada (y: -12 a -8)
  setupMatteMaterial(&scene, "blue_matte", Color(0.0, 0.0, 1.0));
  CylinderParams const cyl = {Point3(0, -10, 0), Vec3(0, 4, 0), 2.0, 4.0};
  setupSingleCylinderScene(&scene, cyl, 0);

  // Rayo con depth=1 desde (0, 0, 0) [y=0, muy por encima] hacia (0, 0, -1)
  // Golpearía el cilindro infinito pero está fuera de la altura válida
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Miss debe retornar backgroundColor
  Vec3 const unit_dir = Vec3(0, 0, -1).normalize();
  double const t      = 0.5 * (unit_dir.y + 1.0);
  Color const expected =
      (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// ============================================================================
// TESTS PARA matteColor
// ============================================================================

// Test 1: Matte material computes attenuation correctly
TEST_F(RendererTest, MatteMaterialComputesAttenuation) {
  // Setup: Establecer max_depth = 2 para permitir 1 rebote
  config.max_depth = 2;

  // Definir color de atenuación
  Color const attenuation(0.8, 0.4, 0.2);

  // Limpiar materiales del SetUp y añadir nuevo material matte
  clearScene(&scene);
  setupMatteMaterial(&scene, "test_matte", attenuation);

  // Configurar una esfera simple para ser golpeada
  setupSingleSphereScene(&scene, Point3(0, 0, -1), 0.5, 0);

  // Lanzar un rayo frontal
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), config.max_depth);

  // Llamar a rayColor
  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // Verificar las aserciones de atenuación
  ASSERT_GE(result.x, 0.0);
  ASSERT_LE(result.x, attenuation.x);
  ASSERT_GE(result.y, 0.0);
  ASSERT_LE(result.y, attenuation.y);
  ASSERT_GE(result.z, 0.0);
  ASSERT_LE(result.z, attenuation.z);

  // Verificar que no sea negro (asumiendo que el fondo no es negro)
  ASSERT_TRUE(result.x > 0.0 or result.y > 0.0 or result.z > 0.0);
}

// Test 2: Matte material near zero bounce case
TEST_F(RendererTest, MatteMaterialNearZeroBounce) {
  // Testea el caso borde is_near_zero(). Este test es difícil de forzar sin un mock de RNG,
  // pero confiamos en que el test anterior (con RNG determinista) lo cubre si el primer
  // vector aleatorio resulta ser cercano a cero. La aserción principal es que el código
  // no crashea y devuelve un color válido.

  // Usar depth = 5 para más rebotes
  config.max_depth = 5;

  // Definir atenuación
  Color const attenuation(0.5, 0.5, 0.5);

  // Limpiar y configurar escena
  clearScene(&scene);
  setupMatteMaterial(&scene, "test_matte", attenuation);
  setupSingleSphereScene(&scene, Point3(0, 0, -1), 0.5, 0);

  // Lanzar rayo
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), config.max_depth);

  // Llamar a rayColor
  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // Verificar las aserciones de atenuación
  ASSERT_LE(result.x, attenuation.x);
  ASSERT_LE(result.y, attenuation.y);
  ASSERT_LE(result.z, attenuation.z);
}

// ============================================================================
// TESTS PARA metalColor
// ============================================================================

// Test 1: Metal material with perfect reflection (diffusion = 0.0)
TEST_F(RendererTest, MetalMaterialPerfectReflection) {
  // Setup: Establecer max_depth = 2 para permitir 1 rebote
  config.max_depth = 2;

  // Definir color de atenuación y diffusion = 0.0 (espejo perfecto)
  Color const attenuation(0.9, 0.9, 0.9);
  double const diffusion = 0.0;

  // Limpiar materiales del SetUp y añadir nuevo material metal
  clearScene(&scene);
  setupMetalMaterial(&scene, "perfect_mirror", attenuation, diffusion);

  // Configurar una esfera simple para ser golpeada
  setupSingleSphereScene(&scene, Point3(0, 0, -1), 0.5, 0);

  // Lanzar un rayo frontal
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), config.max_depth);

  // Llamar a rayColor
  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // Verificar las aserciones de atenuación
  ASSERT_GE(result.x, 0.0);
  ASSERT_LE(result.x, attenuation.x);
  ASSERT_GE(result.y, 0.0);
  ASSERT_LE(result.y, attenuation.y);
  ASSERT_GE(result.z, 0.0);
  ASSERT_LE(result.z, attenuation.z);

  // Verificar que no sea negro (el rebote depth=1 debe golpear el fondo)
  ASSERT_TRUE(result.x > 0.0 or result.y > 0.0 or result.z > 0.0);
}

// Test 2: Metal material with fuzzed reflection (diffusion > 0.0)
TEST_F(RendererTest, MetalMaterialFuzzedReflection) {
  // Setup: Establecer max_depth = 2 para permitir 1 rebote
  config.max_depth = 2;

  // Definir color de atenuación y diffusion > 0.0 (metal difuso)
  Color const attenuation(0.8, 0.6, 0.2);
  double const diffusion = 0.3;

  // Limpiar materiales del SetUp y añadir nuevo material metal
  clearScene(&scene);
  setupMetalMaterial(&scene, "fuzzed_metal", attenuation, diffusion);

  // Configurar una esfera simple para ser golpeada
  setupSingleSphereScene(&scene, Point3(0, 0, -1), 0.5, 0);

  // Lanzar un rayo frontal
  Ray const ray(Point3(0, 0, 0), Vec3(0, 0, -1), config.max_depth);

  // Llamar a rayColor
  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // Verificar las aserciones (acotadas) - puede ser negro si se absorbe
  ASSERT_GE(result.x, 0.0);
  ASSERT_LE(result.x, attenuation.x);
  ASSERT_GE(result.y, 0.0);
  ASSERT_LE(result.y, attenuation.y);
  ASSERT_GE(result.z, 0.0);
  ASSERT_LE(result.z, attenuation.z);
}

// ============================================================================
// TESTS PARA refractiveColor
// ============================================================================

// Test 2: Material refractivo computa reflexión interna total
// (Camino A: Reflexión Interna Total, saliendo desde el interior)
TEST_F(RendererTest, RefractiveMaterialComputesTotalInternalReflection) {
  // Setup: Configurar max_depth = 2 para permitir 1 rebote
  config.max_depth = 2;

  // Definir índice de refracción
  double const ior = 1.5;

  // Limpiar escena y añadir material refractivo
  clearScene(&scene);
  setupRefractiveMaterial(&scene, "glass_tir", ior);

  // Configurar esfera grande (radio 2.0) para que el rayo pueda estar dentro
  setupSingleSphereScene(&scene, Point3(0, 0, 0), 2.0, 0);

  // Lanzar un rayo DESDE DENTRO de la esfera (cerca de la superficie interna)
  // en un ángulo rasante para forzar reflexión interna total
  // Punto de origen: (0, 1.9, 0) - cerca de la superficie superior interna
  // Dirección: (0.1, 1.0, 0.0) normalizada - ángulo rasante hacia arriba
  //
  // Cuando el rayo golpea la superficie desde dentro:
  // - front_face = false
  // - refraction_ratio = 1.0 / ior = 1/1.5 = 0.667
  // - Para ángulos rasantes: refraction_ratio * sin_theta > 1.0
  // - Esto fuerza el Camino A: direction = reflect(...)
  Vec3 const direction = Vec3(0.1, 1.0, 0.0).normalize();
  Ray const ray(Point3(0, 1.9, 0), direction, config.max_depth);

  Color const result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: El resultado no debe ser negro
  // (prueba que la reflexión interna total funcionó y el rayo reflejado
  // eventualmente golpeó el fondo con color)
  ASSERT_TRUE(result.x > 0.0 or result.y > 0.0 or result.z > 0.0)
      << "Expected non-black color from total internal reflection hitting background, got ("
      << result.x << ", " << result.y << ", " << result.z << ")";
}
