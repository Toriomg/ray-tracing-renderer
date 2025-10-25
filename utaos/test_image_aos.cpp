#include "../aos/include/image_aos.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

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

// ============================================================================
// TESTS PARA ImageAOS Getters y Setters
// ============================================================================

// Test 9: Set y Get píxel con índice válido
TEST_F(ImageAOSTest, SetAndGetPixelValidIndex) {
  // Configuración: imagen de 3x2 (6 píxeles, índices 0-5)
  ImageAOS image(3, 2);

  // Crear un píxel de prueba
  Color test_color(0.1, 0.2, 0.3);  // Valores double que serán convertidos

  // Establecer el píxel en el índice 3
  image.set_pixel(3, test_color);

  // Obtener los valores usando los getters individuales
  uint8_t red   = image.get_red(3);
  uint8_t green = image.get_green(3);
  uint8_t blue  = image.get_blue(3);

  // Verificar que los valores se establecieron correctamente
  // Nota: Los valores exactos dependen de la corrección gamma y conversión a uint8_t
  ASSERT_GT(red, 0) << "El componente rojo debe ser mayor que 0";
  ASSERT_GT(green, 0) << "El componente verde debe ser mayor que 0";
  ASSERT_GT(blue, 0) << "El componente azul debe ser mayor que 0";

  // Obtener el píxel completo
  ImageAOS::Pixel retrieved_pixel = image.get_pixel(3);

  // Verificar que get_pixel devuelve los mismos valores que los getters individuales
  ASSERT_EQ(retrieved_pixel.r, red) << "get_pixel().r debe coincidir con get_red()";
  ASSERT_EQ(retrieved_pixel.g, green) << "get_pixel().g debe coincidir con get_green()";
  ASSERT_EQ(retrieved_pixel.b, blue) << "get_pixel().b debe coincidir con get_blue()";
}

// Test 10: Set y Get píxel en el primer índice
TEST_F(ImageAOSTest, SetAndGetPixelFirstIndex) {
  // Configuración
  ImageAOS image(3, 2);

  // Definir color de prueba
  Color test_color(1.0, 1.0, 1.0);  // Blanco (después de gamma debería ser 255, 255, 255)

  // Establecer el píxel en el índice 0
  image.set_pixel(0, test_color);

  // Obtener el píxel
  ImageAOS::Pixel pixel = image.get_pixel(0);

  // Verificar que los valores son correctos (blanco con gamma debería ser 255)
  ASSERT_EQ(pixel.r, 255) << "Primer píxel componente R debe ser 255 (blanco)";
  ASSERT_EQ(pixel.g, 255) << "Primer píxel componente G debe ser 255 (blanco)";
  ASSERT_EQ(pixel.b, 255) << "Primer píxel componente B debe ser 255 (blanco)";

  // Verificar también con los getters individuales
  ASSERT_EQ(image.get_red(0), 255);
  ASSERT_EQ(image.get_green(0), 255);
  ASSERT_EQ(image.get_blue(0), 255);
}

// Test 11: Set y Get píxel en el último índice
TEST_F(ImageAOSTest, SetAndGetPixelLastIndex) {
  // Configuración: imagen de 3x2 (6 píxeles, índice máximo = 5)
  ImageAOS image(3, 2);

  // Definir color de prueba (valores intermedios)
  Color test_color(0.5, 0.5, 0.5);  // Gris medio

  // Establecer el píxel en el último índice (width*height - 1 = 5)
  size_t last_index = image.total_pixels() - 1;
  image.set_pixel(last_index, test_color);

  // Obtener el píxel
  ImageAOS::Pixel pixel = image.get_pixel(last_index);

  // Verificar que los valores están en el rango esperado (no negro)
  ASSERT_GT(pixel.r, 0) << "Último píxel componente R debe ser > 0";
  ASSERT_GT(pixel.g, 0) << "Último píxel componente G debe ser > 0";
  ASSERT_GT(pixel.b, 0) << "Último píxel componente B debe ser > 0";

  // Verificar que no es blanco completo (255)
  ASSERT_LT(pixel.r, 255) << "Último píxel componente R debe ser < 255";
  ASSERT_LT(pixel.g, 255) << "Último píxel componente G debe ser < 255";
  ASSERT_LT(pixel.b, 255) << "Último píxel componente B debe ser < 255";
}

// Test 12: Getters lanzan excepción con índice fuera de rango
TEST_F(ImageAOSTest, GettersThrowOutOfRange) {
  // Configuración: imagen de 3x2 (6 píxeles, índices válidos: 0-5)
  ImageAOS image(3, 2);

  // Índice inválido (igual a width*height)
  size_t invalid_index = image.total_pixels();

  // Verificar que todos los getters lanzan std::out_of_range
  ASSERT_THROW((void) image.get_red(invalid_index), std::out_of_range)
      << "get_red() debe lanzar std::out_of_range con índice fuera de rango";

  ASSERT_THROW((void) image.get_green(invalid_index), std::out_of_range)
      << "get_green() debe lanzar std::out_of_range con índice fuera de rango";

  ASSERT_THROW((void) image.get_blue(invalid_index), std::out_of_range)
      << "get_blue() debe lanzar std::out_of_range con índice fuera de rango";

  ASSERT_THROW((void) image.get_pixel(invalid_index), std::out_of_range)
      << "get_pixel() debe lanzar std::out_of_range con índice fuera de rango";
}

// Test 13: set_pixel lanza excepción con índice fuera de rango
TEST_F(ImageAOSTest, SetPixelThrowsOutOfRange) {
  // Configuración: imagen de 3x2 (6 píxeles, índices válidos: 0-5)
  ImageAOS image(3, 2);

  // Índice inválido
  size_t invalid_index = image.total_pixels();

  // Color de prueba
  Color test_color(0.5, 0.5, 0.5);

  // Verificar que set_pixel lanza std::out_of_range
  ASSERT_THROW(image.set_pixel(invalid_index, test_color), std::out_of_range)
      << "set_pixel() debe lanzar std::out_of_range con índice fuera de rango";
}

// Test 14: Verificar que set_pixel no afecta otros píxeles
TEST_F(ImageAOSTest, SetPixelDoesNotAffectOtherPixels) {
  // Configuración
  ImageAOS image(3, 2);

  // Establecer un píxel específico
  Color test_color(1.0, 0.0, 0.0);  // Rojo
  image.set_pixel(2, test_color);

  // Verificar que otros píxeles siguen siendo negros (0, 0, 0)
  for (size_t i = 0; i < image.total_pixels(); ++i) {
    if (i == 2) {
      // El píxel 2 debe ser rojo
      ASSERT_EQ(image.get_red(i), 255) << "Píxel 2 debe tener R=255";
      ASSERT_EQ(image.get_green(i), 0) << "Píxel 2 debe tener G=0";
      ASSERT_EQ(image.get_blue(i), 0) << "Píxel 2 debe tener B=0";
    } else {
      // Todos los demás píxeles deben seguir siendo negros
      ImageAOS::Pixel pixel = image.get_pixel(i);
      ASSERT_EQ(pixel.r, 0) << "Píxel " << i << " componente R debe ser 0";
      ASSERT_EQ(pixel.g, 0) << "Píxel " << i << " componente G debe ser 0";
      ASSERT_EQ(pixel.b, 0) << "Píxel " << i << " componente B debe ser 0";
    }
  }
}

// Test 15: Getters con índice extremo muy grande
TEST_F(ImageAOSTest, GettersThrowWithLargeIndex) {
  // Configuración
  ImageAOS image(3, 2);

  // Índice extremadamente grande
  size_t huge_index = 1'000'000;

  // Verificar que lanzan excepción
  ASSERT_THROW((void) image.get_red(huge_index), std::out_of_range);
  ASSERT_THROW((void) image.get_green(huge_index), std::out_of_range);
  ASSERT_THROW((void) image.get_blue(huge_index), std::out_of_range);
  ASSERT_THROW((void) image.get_pixel(huge_index), std::out_of_range);
}

// Test 16: Múltiples operaciones set_pixel
TEST_F(ImageAOSTest, MultipleSetPixelOperations) {
  // Configuración
  ImageAOS image(3, 2);

  // Establecer varios píxeles con diferentes colores
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde
  image.set_pixel(2, Color(0.0, 0.0, 1.0));  // Azul
  image.set_pixel(3, Color(1.0, 1.0, 0.0));  // Amarillo
  image.set_pixel(4, Color(1.0, 0.0, 1.0));  // Magenta
  image.set_pixel(5, Color(0.0, 1.0, 1.0));  // Cian

  // Verificar cada píxel
  // Píxel 0: Rojo
  ASSERT_EQ(image.get_red(0), 255);
  ASSERT_EQ(image.get_green(0), 0);
  ASSERT_EQ(image.get_blue(0), 0);

  // Píxel 1: Verde
  ASSERT_EQ(image.get_red(1), 0);
  ASSERT_EQ(image.get_green(1), 255);
  ASSERT_EQ(image.get_blue(1), 0);

  // Píxel 2: Azul
  ASSERT_EQ(image.get_red(2), 0);
  ASSERT_EQ(image.get_green(2), 0);
  ASSERT_EQ(image.get_blue(2), 255);

  // Píxel 3: Amarillo
  ASSERT_EQ(image.get_red(3), 255);
  ASSERT_EQ(image.get_green(3), 255);
  ASSERT_EQ(image.get_blue(3), 0);

  // Píxel 4: Magenta
  ASSERT_EQ(image.get_red(4), 255);
  ASSERT_EQ(image.get_green(4), 0);
  ASSERT_EQ(image.get_blue(4), 255);

  // Píxel 5: Cian
  ASSERT_EQ(image.get_red(5), 0);
  ASSERT_EQ(image.get_green(5), 255);
  ASSERT_EQ(image.get_blue(5), 255);
}

// ============================================================================
// TESTS PARA ImageAOS::fill_from_double
// ============================================================================

// Test 17: fill_from_double con datos válidos
TEST_F(ImageAOSTest, FillFromDoubleValidData) {
  // Configuración: imagen de 1x2 (2 píxeles)
  ImageAOS image(1, 2);

  // Gamma personalizado
  double gamma = 2.0;

  // Datos de entrada (tamaño = 2)
  std::vector<double> r_data = {0.25, 0.5};
  std::vector<double> g_data = {0.5, 0.75};
  std::vector<double> b_data = {1.0, 0.0};

  // Llamar a fill_from_double
  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Calcular valores esperados usando la misma fórmula: uint8_t = 255.999 * pow(value, 1/gamma)
  // Píxel 0: r=0.25, g=0.5, b=1.0
  auto expected_r0 = static_cast<uint8_t>(255.999 * std::pow(0.25, 1.0 / gamma));
  auto expected_g0 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_b0 = static_cast<uint8_t>(255.999 * std::pow(1.0, 1.0 / gamma));

  // Píxel 1: r=0.5, g=0.75, b=0.0
  auto expected_r1 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_g1 = static_cast<uint8_t>(255.999 * std::pow(0.75, 1.0 / gamma));
  auto expected_b1 = static_cast<uint8_t>(255.999 * std::pow(0.0, 1.0 / gamma));

  // Verificar Píxel 0
  ImageAOS::Pixel p0 = image.get_pixel(0);
  ASSERT_EQ(p0.r, expected_r0) << "Píxel 0: componente R incorrecto";
  ASSERT_EQ(p0.g, expected_g0) << "Píxel 0: componente G incorrecto";
  ASSERT_EQ(p0.b, expected_b0) << "Píxel 0: componente B incorrecto (debería ser 255)";

  // Verificar Píxel 1
  ImageAOS::Pixel p1 = image.get_pixel(1);
  ASSERT_EQ(p1.r, expected_r1) << "Píxel 1: componente R incorrecto";
  ASSERT_EQ(p1.g, expected_g1) << "Píxel 1: componente G incorrecto";
  ASSERT_EQ(p1.b, expected_b1) << "Píxel 1: componente B incorrecto (debería ser 0)";

  // Verificación adicional: b0 debe ser 255 (valor máximo) y b1 debe ser 0
  ASSERT_EQ(p0.b, 255) << "Valor 1.0 con gamma debería resultar en 255";
  ASSERT_EQ(p1.b, 0) << "Valor 0.0 con gamma debería resultar en 0";
}

// Test 18: fill_from_double con gamma por defecto
TEST_F(ImageAOSTest, FillFromDoubleDefaultGamma) {
  // Configuración: imagen de 1x1
  ImageAOS image(1, 1);

  // Datos de entrada
  std::vector<double> r_data = {0.5};
  std::vector<double> g_data = {0.5};
  std::vector<double> b_data = {0.5};

  // Llamar a fill_from_double SIN especificar gamma (usa el default)
  image.fill_from_double(r_data, g_data, b_data);

  // Calcular valor esperado con gamma por defecto (Constants::Gamma)
  // Asumiendo que el default es 2.2 según constants.hpp
  double default_gamma = 2.2;  // Constants::Gamma
  auto expected        = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));

  // Verificar píxel
  ImageAOS::Pixel p = image.get_pixel(0);
  ASSERT_EQ(p.r, expected) << "Componente R con gamma default incorrecto";
  ASSERT_EQ(p.g, expected) << "Componente G con gamma default incorrecto";
  ASSERT_EQ(p.b, expected) << "Componente B con gamma default incorrecto";
}

// Test 19: fill_from_double lanza excepción con tamaño inválido
TEST_F(ImageAOSTest, FillFromDoubleThrowsOnInvalidSize) {
  // Configuración: imagen de 3x3 (9 píxeles)
  ImageAOS image(3, 3);

  // Datos de entrada con tamaño INCORRECTO (solo 1 elemento en lugar de 9)
  std::vector<double> r_data = {0.5};
  std::vector<double> g_data = {0.5};
  std::vector<double> b_data = {0.5};

  // Verificar que se lanza std::invalid_argument
  ASSERT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument)
      << "fill_from_double debe lanzar std::invalid_argument cuando el tamaño de los datos no "
         "coincide";
}

// Test 20: fill_from_double con un canal de tamaño incorrecto
TEST_F(ImageAOSTest, FillFromDoubleThrowsOnOneChannelWrongSize) {
  // Configuración: imagen de 2x2 (4 píxeles)
  ImageAOS image(2, 2);

  // Canal R correcto (4 elementos)
  std::vector<double> r_data = {0.1, 0.2, 0.3, 0.4};
  // Canal G correcto (4 elementos)
  std::vector<double> g_data = {0.5, 0.6, 0.7, 0.8};
  // Canal B INCORRECTO (solo 3 elementos)
  std::vector<double> b_data = {0.9, 1.0, 0.5};

  // Verificar que se lanza std::invalid_argument
  ASSERT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument)
      << "fill_from_double debe lanzar std::invalid_argument cuando un canal tiene tamaño "
         "incorrecto";
}

// Test 21: fill_from_double con valores extremos
TEST_F(ImageAOSTest, FillFromDoubleExtremeValues) {
  // Configuración: imagen de 3x1
  ImageAOS image(3, 1);

  double gamma = 2.0;

  // Datos con valores extremos: 0.0, 1.0, y un valor intermedio
  std::vector<double> r_data = {0.0, 1.0, 0.5};
  std::vector<double> g_data = {1.0, 0.0, 0.5};
  std::vector<double> b_data = {0.5, 0.5, 1.0};

  // Llamar a fill_from_double
  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Verificar píxel 0: r=0.0 (negro), g=1.0 (blanco), b=0.5
  ImageAOS::Pixel p0 = image.get_pixel(0);
  ASSERT_EQ(p0.r, 0) << "Valor 0.0 debe resultar en 0";
  ASSERT_EQ(p0.g, 255) << "Valor 1.0 debe resultar en 255";

  // Verificar píxel 1: r=1.0 (blanco), g=0.0 (negro), b=0.5
  ImageAOS::Pixel p1 = image.get_pixel(1);
  ASSERT_EQ(p1.r, 255) << "Valor 1.0 debe resultar en 255";
  ASSERT_EQ(p1.g, 0) << "Valor 0.0 debe resultar en 0";

  // Verificar píxel 2: todos los valores 0.5 o 1.0
  ImageAOS::Pixel p2 = image.get_pixel(2);
  auto expected_05   = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  ASSERT_EQ(p2.r, expected_05) << "Píxel 2: componente R incorrecto";
  ASSERT_EQ(p2.g, expected_05) << "Píxel 2: componente G incorrecto";
  ASSERT_EQ(p2.b, 255) << "Píxel 2: componente B debe ser 255";
}

// Test 22: fill_from_double sobrescribe píxeles anteriores
TEST_F(ImageAOSTest, FillFromDoubleOverwritesPreviousData) {
  // Configuración
  ImageAOS image(2, 1);

  // Primero, establecer píxeles manualmente
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde

  // Verificar que se establecieron
  ASSERT_EQ(image.get_red(0), 255);
  ASSERT_EQ(image.get_green(1), 255);

  // Ahora llamar a fill_from_double con datos diferentes
  std::vector<double> r_data = {0.0, 0.0};
  std::vector<double> g_data = {0.0, 0.0};
  std::vector<double> b_data = {1.0, 1.0};  // Azul

  image.fill_from_double(r_data, g_data, b_data);

  // Verificar que los píxeles fueron sobrescritos
  ImageAOS::Pixel p0 = image.get_pixel(0);
  ImageAOS::Pixel p1 = image.get_pixel(1);

  ASSERT_EQ(p0.r, 0) << "Píxel 0 R debe haber sido sobrescrito a 0";
  ASSERT_EQ(p0.g, 0) << "Píxel 0 G debe haber sido sobrescrito a 0";
  ASSERT_EQ(p0.b, 255) << "Píxel 0 B debe haber sido sobrescrito a 255";

  ASSERT_EQ(p1.r, 0) << "Píxel 1 R debe haber sido sobrescrito a 0";
  ASSERT_EQ(p1.g, 0) << "Píxel 1 G debe haber sido sobrescrito a 0";
  ASSERT_EQ(p1.b, 255) << "Píxel 1 B debe haber sido sobrescrito a 255";
}

// Test 23: fill_from_double con imagen grande
TEST_F(ImageAOSTest, FillFromDoubleLargeImage) {
  // Configuración: imagen más grande (10x10 = 100 píxeles)
  size_t size = 100;
  ImageAOS image(10, 10);

  // Crear datos de entrada (todos con valor 0.5)
  std::vector<double> r_data(size, 0.5);
  std::vector<double> g_data(size, 0.5);
  std::vector<double> b_data(size, 0.5);

  double gamma = 2.0;

  // Llamar a fill_from_double
  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Calcular valor esperado
  auto expected = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));

  // Verificar algunos píxeles aleatorios
  ASSERT_EQ(image.get_pixel(0).r, expected);
  ASSERT_EQ(image.get_pixel(50).g, expected);
  ASSERT_EQ(image.get_pixel(99).b, expected);

  // Verificar que todos los píxeles tienen el mismo valor
  for (size_t i = 0; i < size; ++i) {
    ImageAOS::Pixel p = image.get_pixel(i);
    ASSERT_EQ(p.r, expected) << "Píxel " << i << " componente R incorrecto";
    ASSERT_EQ(p.g, expected) << "Píxel " << i << " componente G incorrecto";
    ASSERT_EQ(p.b, expected) << "Píxel " << i << " componente B incorrecto";
  }
}
