#include "dataStructs/settings_structs.hpp"
#include "ray.hpp"
#include "renderer.hpp"
#include "utilities/random.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <gtest/gtest.h>

// ============================================================================
// ESTRATEGIA DE TESTING "CAJA NEGRA" ROBUSTA
// ============================================================================
// En lugar de testear RenderSpheres (privado) directamente, testeamos a través
// de rayColor (API pública) explotando el caso base de la recursión (depth=0).
//
// ESTRATEGIA:
// - Si un rayo con depth=1 GOLPEA una esfera:
//   * rayColor llama a RenderSpheres -> encuentra hit
//   * Llama a matteColor (u otro material)
//   * matteColor crea rayo rebotado con depth=0
//   * rayColor(depth=0) retorna Color(0,0,0) [caso base]
//   * Resultado final: Color(0,0,0)
//
// - Si un rayo con depth=1 NO GOLPEA:
//   * RenderSpheres retorna nullopt
//   * rayColor retorna backgroundColor
//
// VENTAJAS:
// - Aserción binaria y fuerte (negro vs fondo)
// - No modifica código de producción
// - Testea la lógica de intersección indirectamente
// - Robusto ante cambios internos
// ============================================================================

namespace {

  void setupSingleSphereScene(SceneSettings & scene, Point3 center, double radius,
                              unsigned int mat_id) {
    scene.spheres.x.push_back(center.x);
    scene.spheres.y.push_back(center.y);
    scene.spheres.z.push_back(center.z);
    scene.spheres.r.push_back(radius);
    scene.spheres.materialIndex.push_back(mat_id);

    // Generar AABB para la esfera
    scene.spheres.aabbs.push_back(AABB::from_sphere(center, radius));
  }

  void setupMatteMaterial(SceneSettings & scene, std::string const & name, Color color) {
    scene.materialNames.push_back(name);
    scene.matte.r.push_back(color.x);
    scene.matte.g.push_back(color.y);
    scene.matte.b.push_back(color.z);

    auto local_index = static_cast<unsigned int>(scene.matte.r.size() - 1);
    scene.materialTable.push_back({MaterialType::MATTE, local_index});
  }

  void clearScene(SceneSettings & scene) {
    scene.spheres.x.clear();
    scene.spheres.y.clear();
    scene.spheres.z.clear();
    scene.spheres.r.clear();
    scene.spheres.materialIndex.clear();
    scene.spheres.aabbs.clear();

    scene.cylinders.x.clear();
    scene.cylinders.y.clear();
    scene.cylinders.z.clear();
    scene.cylinders.r.clear();
    scene.cylinders.vx.clear();
    scene.cylinders.vy.clear();
    scene.cylinders.vz.clear();
    scene.cylinders.invAxisLen.clear();
    scene.cylinders.materialIndex.clear();
    scene.cylinders.aabbs.clear();

    scene.materialNames.clear();
    scene.materialTable.clear();
    scene.matte.r.clear();
    scene.matte.g.clear();
    scene.matte.b.clear();
    scene.metal.r.clear();
    scene.metal.g.clear();
    scene.metal.b.clear();
    scene.metal.diffusion.clear();
    scene.refractive.ior.clear();
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

  void SetUp() override {
    clearScene(scene);

    // Configuración por defecto
    config.image_width            = 100;
    config.aspect_ratio           = {16, 9};
    config.samples_per_pixel      = 1;
    config.max_depth              = 5;
    config.background_dark_color  = Color(0.5, 0.7, 1.0);
    config.background_light_color = Color(1.0, 1.0, 1.0);

    // Inicializar RNG con seed fija
    rng = RandomGenerator(12'345);
  }

  void TearDown() override { clearScene(scene); }
};

// ============================================================================
// TESTS PARA RenderSpheres
// ============================================================================

// Casos de ACIERTO

// Test 1: Hit Frontal - Rayo golpea esfera de frente
TEST_F(RendererTest, RayColorHitsFrontSphere) {
  // Setup: Esfera roja en (0, 0, -5) con radio 1.0
  setupMatteMaterial(scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 desde origen hacia -Z (debe golpear en t≈4.0)
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  // Llamar a rayColor (internamente usa RenderSpheres)
  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 4: Rayo desde dentro de la esfera
TEST_F(RendererTest, RayColorFromInsideSphere) {
  // Setup: Esfera grande en origen con radio 2.0
  setupMatteMaterial(scene, "green_matte", Color(0.0, 1.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, 0), 2.0, 0);

  // Rayo con depth=1 desde el centro hacia fuera
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, 1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Hit desde dentro con depth=1 debe dar negro
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 5: Múltiples esferas - golpea la más cercana
TEST_F(RendererTest, RayColorHitsClosestSphere) {
  // Setup: Dos esferas en la misma línea
  setupMatteMaterial(scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupMatteMaterial(scene, "blue_matte", Color(0.0, 0.0, 1.0));

  setupSingleSphereScene(scene, Point3(0, 0, -3), 0.5, 0);  // Roja más cercana
  setupSingleSphereScene(scene, Point3(0, 0, -6), 0.5, 1);  // Azul más lejos

  // Rayo con depth=1
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera más cercana
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 7: Rayo tangente a esfera
TEST_F(RendererTest, RayColorTangentToSphere) {
  // Setup: Esfera en (0, 0, -5) con radio 1.0
  setupMatteMaterial(scene, "yellow_matte", Color(1.0, 1.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo tangente con depth=1 desde (1, 0, 0) hacia -Z
  Ray ray(Point3(1, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Tangente debe golpear (discriminante ≈ 0 válido)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 9: Esfera muy pequeña
TEST_F(RendererTest, RayColorHitsVerySmallSphere) {
  setupMatteMaterial(scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 0.001, 0);

  // Rayo con depth=1 que pasa por el centro
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear esfera pequeña
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 10: Rayo diagonal
TEST_F(RendererTest, RayColorDiagonalRay) {
  setupMatteMaterial(scene, "cyan_matte", Color(0.0, 1.0, 1.0));
  setupSingleSphereScene(scene, Point3(1, 1, -5), 1.0, 0);

  // Rayo diagonal con depth=1
  Vec3 direction = Vec3(1, 1, -5).normalize();
  Ray ray(Point3(0, 0, 0), direction, 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// TESTS ROBUSTOS - Casos de FALLO (Miss)

// Test 2: Miss - Rayo pasa de largo
TEST_F(RendererTest, RayColorMissesSphere) {
  // Setup: Esfera en (0, 0, -5) con radio 1.0
  setupMatteMaterial(scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 que pasa de largo (dirección +Y)
  Ray ray(Point3(0, 0, 0), Vec3(0, 1, 0), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Miss debe retornar backgroundColor
  // Para direccinnn (0, 1, 0) normalizada:
  Vec3 unit_dir  = Vec3(0, 1, 0).normalize();
  double t       = 0.5 * (unit_dir.y + 1.0);
  Color expected = (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// Test 3: Esfera detrás del rayo
TEST_F(RendererTest, RayColorSphereBehindRay) {
  // Setup: Esfera en (0, 0, -5)
  setupMatteMaterial(scene, "blue_matte", Color(0.0, 0.0, 1.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 apuntando en dirección opuesta (+Z)
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, 1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Esfera detrás debe dar backgroundColor
  Vec3 unit_dir  = Vec3(0, 0, 1).normalize();
  double t       = 0.5 * (unit_dir.y + 1.0);
  Color expected = (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// Test 6: Escena vacía - solo fondo
TEST_F(RendererTest, RayColorEmptyScene) {
  // No añadir ninguna esfera

  // Rayo con depth=1 en dirección arbitraria
  Vec3 direction = Vec3(0, 0.5, -1).normalize();
  Ray ray(Point3(0, 0, 0), direction, 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Sin esferas debe retornar backgroundColor
  double t       = 0.5 * (direction.y + 1.0);
  Color expected = (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// Test 11: Rayo casi tangente (miss por poco)
TEST_F(RendererTest, RayColorJustMissesSphere) {
  setupMatteMaterial(scene, "red_matte", Color(1.0, 0.0, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 que pasa justo afuera (x=1.1, radio=1.0)
  Ray ray(Point3(1.1, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Miss debe retornar backgroundColor
  Vec3 unit_dir  = Vec3(0, 0, -1).normalize();
  double t       = 0.5 * (unit_dir.y + 1.0);
  Color expected = (1.0 - t) * config.background_dark_color + t * config.background_light_color;

  ASSERT_NEAR(result.x, expected.x, 0.01);
  ASSERT_NEAR(result.y, expected.y, 0.01);
  ASSERT_NEAR(result.z, expected.z, 0.01);
}

// TEST ESPECIAL - Verifica el caso base (max_depth = 0)

// Test 8: Depth limit - verificar que max_depth funciona
TEST_F(RendererTest, RayColorRespectsMaxDepth) {
  // Setup: Esfera con material matte
  setupMatteMaterial(scene, "gray_matte", Color(0.5, 0.5, 0.5));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Configurar max_depth = 0 (sin rebotes)
  config.max_depth = 0;

  // Rayo sin especificar depth (usa constructor por defecto, depth=0)
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1), 0);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // Con depth = 0, debe retornar negro inmediatamente (caso base)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// TESTS ADICIONALES - Casos extremos

// Test 12: Esfera en el origen
TEST_F(RendererTest, RayColorSphereAtOrigin) {
  setupMatteMaterial(scene, "magenta_matte", Color(1.0, 0.0, 1.0));
  setupSingleSphereScene(scene, Point3(0, 0, 0), 1.0, 0);

  // Rayo con depth=1 desde (-5, 0, 0) hacia +X
  Ray ray(Point3(-5, 0, 0), Vec3(1, 0, 0), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear la esfera en el origen
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 13: Esfera muy lejos
TEST_F(RendererTest, RayColorSphereVeryFar) {
  setupMatteMaterial(scene, "orange_matte", Color(1.0, 0.5, 0.0));
  setupSingleSphereScene(scene, Point3(0, 0, -1'000), 10.0, 0);

  // Rayo con depth=1 desde origen hacia -Z
  Ray ray(Point3(0, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear incluso estando muy lejos
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

// Test 14: Rayo descentrado
TEST_F(RendererTest, RayColorOffCenterHit) {
  setupMatteMaterial(scene, "purple_matte", Color(0.5, 0.0, 0.5));
  setupSingleSphereScene(scene, Point3(0, 0, -5), 1.0, 0);

  // Rayo con depth=1 desde (0.5, 0, 0) hacia -Z (golpea descentrado)
  Ray ray(Point3(0.5, 0, 0), Vec3(0, 0, -1), 1);

  Color result = Renderer::rayColor(ray, scene, config, rng);

  // ASERCIÓN ROBUSTA: Debe golpear (0.5 < radio 1.0)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}
