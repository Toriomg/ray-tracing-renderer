#include "../aos/include/image_aos.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA ImageAOS
// ============================================================================

class ImageAOSTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza común si es necesaria
  }
};

// ============================================================================
// TESTS PARA ImageAOS::ImageAOS (Constructor)
// ============================================================================

// Test 1: Constructor con dimensiones válidas
TEST_F(ImageAOSTest, ConstructorValidDimensions) {
  // Configuración
  size_t width  = 10;
  size_t height = 5;

  // Llamar al constructor
  ImageAOS image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), width) << "El ancho de la imagen debe ser " << width;
  ASSERT_EQ(image.height(), height) << "El alto de la imagen debe ser " << height;
  ASSERT_EQ(image.total_pixels(), width * height)
      << "El número total de píxeles debe ser width * height";

  // Verificar que los píxeles se inicializan a negro (0, 0, 0)
  ImageAOS::Pixel pixel = image.get_pixel(0);
  ASSERT_EQ(pixel.r, 0) << "El componente rojo inicial debe ser 0";
  ASSERT_EQ(pixel.g, 0) << "El componente verde inicial debe ser 0";
  ASSERT_EQ(pixel.b, 0) << "El componente azul inicial debe ser 0";

  // Verificar que todos los píxeles están inicializados
  ASSERT_EQ(image.get_pixels().size(), width * height)
      << "El vector de píxeles debe tener el tamaño correcto";
}

// Test 2: Constructor lanza excepción con ancho cero
TEST_F(ImageAOSTest, ConstructorThrowsOnZeroWidth) {
  // Configuración
  size_t width  = 0;
  size_t height = 5;

  // Verificar que se lanza std::invalid_argument
  ASSERT_THROW(ImageAOS(width, height), std::invalid_argument)
      << "Constructor debe lanzar std::invalid_argument cuando width es 0";
}

// Test 3: Constructor lanza excepción con alto cero
TEST_F(ImageAOSTest, ConstructorThrowsOnZeroHeight) {
  // Configuración
  size_t width  = 10;
  size_t height = 0;

  // Verificar que se lanza std::invalid_argument
  ASSERT_THROW(ImageAOS(width, height), std::invalid_argument)
      << "Constructor debe lanzar std::invalid_argument cuando height es 0";
}

// Test 4: Constructor con dimensiones 1x1 (caso mínimo válido)
TEST_F(ImageAOSTest, ConstructorMinimalDimensions) {
  // Configuración: imagen de 1x1 píxel
  size_t width  = 1;
  size_t height = 1;

  // Llamar al constructor
  ImageAOS image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), 1);
  ASSERT_EQ(image.height(), 1);
  ASSERT_EQ(image.total_pixels(), 1);

  // Verificar que el único píxel existe y está inicializado a negro
  ImageAOS::Pixel pixel = image.get_pixel(0);
  ASSERT_EQ(pixel.r, 0);
  ASSERT_EQ(pixel.g, 0);
  ASSERT_EQ(pixel.b, 0);
}

// Test 5: Constructor con dimensiones grandes
TEST_F(ImageAOSTest, ConstructorLargeDimensions) {
  // Configuración: imagen grande (ej. Full HD)
  size_t width  = 1'920;
  size_t height = 1'080;

  // Llamar al constructor
  ImageAOS image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), width);
  ASSERT_EQ(image.height(), height);
  ASSERT_EQ(image.total_pixels(), width * height);

  // Verificar que el vector de píxeles tiene el tamaño correcto
  ASSERT_EQ(image.get_pixels().size(), 1'920UL * 1'080UL);
}

// Test 6: Constructor con ambas dimensiones cero
TEST_F(ImageAOSTest, ConstructorThrowsOnBothZero) {
  // Configuración
  size_t width  = 0;
  size_t height = 0;

  // Verificar que se lanza std::invalid_argument
  ASSERT_THROW(ImageAOS(width, height), std::invalid_argument)
      << "Constructor debe lanzar std::invalid_argument cuando ambas dimensiones son 0";
}

// Test 7: Verificar que todos los píxeles están inicializados a negro
TEST_F(ImageAOSTest, ConstructorInitializesAllPixelsToBlack) {
  // Configuración
  size_t width  = 5;
  size_t height = 5;

  // Llamar al constructor
  ImageAOS image(width, height);

  // Verificar que todos los píxeles están en negro (0, 0, 0)
  for (size_t i = 0; i < image.total_pixels(); ++i) {
    ImageAOS::Pixel pixel = image.get_pixel(i);
    ASSERT_EQ(pixel.r, 0) << "Píxel " << i << " componente R debe ser 0";
    ASSERT_EQ(pixel.g, 0) << "Píxel " << i << " componente G debe ser 0";
    ASSERT_EQ(pixel.b, 0) << "Píxel " << i << " componente B debe ser 0";
  }
}

// Test 8: Verificar dimensiones no cuadradas
TEST_F(ImageAOSTest, ConstructorNonSquareDimensions) {
  // Configuración: imagen rectangular (no cuadrada)
  size_t width  = 16;
  size_t height = 9;

  // Llamar al constructor
  ImageAOS image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), 16);
  ASSERT_EQ(image.height(), 9);
  ASSERT_EQ(image.total_pixels(), 144);

  // Verificar que get_pixels() devuelve el tamaño correcto
  ASSERT_EQ(image.get_pixels().size(), 144UL);
}
