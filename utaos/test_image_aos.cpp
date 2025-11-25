#include "../aos/include/image_aos.hpp"
#include "utilities/vec3.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA ImageAOS
// ============================================================================

class ImageAOSTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza de archivos temporales creados durante los tests
    static_cast<void>(std::remove("test_aos.ppm"));
    static_cast<void>(std::remove("test_error_aos.ppm"));
  }
};

// ============================================================================
// TESTS PARA ImageAOS (Constructor)
// ============================================================================

// Test 1: Constructor con dimensiones válidas
TEST_F(ImageAOSTest, ConstructorValidDimensions) {
  // Configuración
  size_t const width  = 10;
  size_t const height = 5;

  // Llamar al constructor
  ImageAOS const image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), width) << "El ancho de la imagen debe ser " << width;
  ASSERT_EQ(image.height(), height) << "El alto de la imagen debe ser " << height;
  // Verificar que los píxeles se inicializan a negro (0, 0, 0)
  ImageAOS::Pixel const pixel = image.get_pixel(0);
  ASSERT_EQ(pixel.r, 0) << "El componente rojo inicial debe ser 0";
  ASSERT_EQ(pixel.g, 0) << "El componente verde inicial debe ser 0";
  ASSERT_EQ(pixel.b, 0) << "El componente azul inicial debe ser 0";
}

// Test 4: Constructor con dimensiones 1x1 (caso mínimo válido)
TEST_F(ImageAOSTest, ConstructorMinimalDimensions) {
  // Configuración: imagen de 1x1 píxel
  size_t const width  = 1;
  size_t const height = 1;

  // Llamar al constructor
  ImageAOS const image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), 1);
  ASSERT_EQ(image.height(), 1);
  ASSERT_EQ(image.height() * image.width(), 1);

  // Verificar que el único píxel existe y está inicializado a negro
  ImageAOS::Pixel const pixel = image.get_pixel(0);
  ASSERT_EQ(pixel.r, 0);
  ASSERT_EQ(pixel.g, 0);
  ASSERT_EQ(pixel.b, 0);
}

// Test 5: Constructor con dimensiones grandes
TEST_F(ImageAOSTest, ConstructorLargeDimensions) {
  // Configuración: imagen grande (ej. Full HD)
  size_t const width  = 1'920;
  size_t const height = 1'080;

  // Llamar al constructor
  ImageAOS const image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), width);
  ASSERT_EQ(image.height(), height);
}

// Test 7: Verificar que todos los píxeles están inicializados a negro
TEST_F(ImageAOSTest, ConstructorInitializesAllPixelsToBlack) {
  // Configuración
  size_t const width  = 5;
  size_t const height = 5;

  // Llamar al constructor
  ImageAOS const image(width, height);

  // Verificar que todos los píxeles están en negro (0, 0, 0)
  for (size_t i = 0; i < image.height() * image.width(); ++i) {
    ImageAOS::Pixel const pixel = image.get_pixel(i);
    ASSERT_EQ(pixel.r, 0) << "Píxel " << i << " componente R debe ser 0";
    ASSERT_EQ(pixel.g, 0) << "Píxel " << i << " componente G debe ser 0";
    ASSERT_EQ(pixel.b, 0) << "Píxel " << i << " componente B debe ser 0";
  }
}

// Test 8: Verificar dimensiones no cuadradas
TEST_F(ImageAOSTest, ConstructorNonSquareDimensions) {
  // Configuración: imagen rectangular (no cuadrada)
  size_t const width  = 16;
  size_t const height = 9;

  // Llamar al constructor
  ImageAOS const image(width, height);

  // Verificar dimensiones
  ASSERT_EQ(image.width(), 16);
  ASSERT_EQ(image.height(), 9);
  ASSERT_EQ(image.width() * image.height(), 144);
}

// ============================================================================
// TESTS PARA Getters y Setters
// ============================================================================

// Test 9: Set y Get píxel con índice válido
TEST_F(ImageAOSTest, SetAndGetPixelValidIndex) {
  // Configuración: imagen de 3x2 (6 píxeles, índices 0-5)
  ImageAOS image(3, 2);

  // Crear un píxel de prueba
  Color const test_color(0.1, 0.2, 0.3);  // Valores double que serán convertidos

  // Establecer el píxel en el índice 3
  image.set_pixel(3, test_color);

  ImageAOS::Pixel const pixel = image.get_pixel(3);
  uint8_t const red           = pixel.r;
  uint8_t const green         = pixel.g;
  uint8_t const blue          = pixel.b;

  // Verificar que los valores se establecieron correctamente
  // Nota: Los valores exactos dependen de la corrección gamma y conversión a uint8_t
  ASSERT_GT(red, 0) << "El componente rojo debe ser mayor que 0";
  ASSERT_GT(green, 0) << "El componente verde debe ser mayor que 0";
  ASSERT_GT(blue, 0) << "El componente azul debe ser mayor que 0";

  // Obtener el píxel completo
  ImageAOS::Pixel const retrieved_pixel = image.get_pixel(3);

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
  Color const test_color(1.0, 1.0, 1.0);  // Blanco (después de gamma debería ser 255, 255, 255)

  // Establecer el píxel en el índice 0
  image.set_pixel(0, test_color);

  // Obtener el píxel
  ImageAOS::Pixel const pixel = image.get_pixel(0);

  // Verificar que los valores son correctos (blanco con gamma debería ser 255)
  ASSERT_EQ(pixel.r, 255) << "Primer píxel componente R debe ser 255 (blanco)";
  ASSERT_EQ(pixel.g, 255) << "Primer píxel componente G debe ser 255 (blanco)";
  ASSERT_EQ(pixel.b, 255) << "Primer píxel componente B debe ser 255 (blanco)";
}

// Test 11: Set y Get píxel en el último índice
TEST_F(ImageAOSTest, SetAndGetPixelLastIndex) {
  // Configuración: imagen de 3x2 (6 píxeles, índice máximo = 5)
  ImageAOS image(3, 2);

  // Definir color de prueba (valores intermedios)
  Color const test_color(0.5, 0.5, 0.5);  // Gris medio

  // Establecer el píxel en el último índice (width*height - 1 = 5)
  size_t const last_index = (image.width() * image.height()) - 1;
  image.set_pixel(last_index, test_color);

  // Obtener el píxel
  ImageAOS::Pixel const pixel = image.get_pixel(last_index);

  // Verificar que los valores están en el rango esperado (no negro)
  ASSERT_GT(pixel.r, 0) << "Último píxel componente R debe ser > 0";
  ASSERT_GT(pixel.g, 0) << "Último píxel componente G debe ser > 0";
  ASSERT_GT(pixel.b, 0) << "Último píxel componente B debe ser > 0";

  // Verificar que no es blanco completo (255)
  ASSERT_LT(pixel.r, 255) << "Último píxel componente R debe ser < 255";
  ASSERT_LT(pixel.g, 255) << "Último píxel componente G debe ser < 255";
  ASSERT_LT(pixel.b, 255) << "Último píxel componente B debe ser < 255";
}

// Test 14: Verificar que set_pixel no afecta otros píxeles
TEST_F(ImageAOSTest, SetPixelDoesNotAffectOtherPixels) {
  // Configuración
  ImageAOS image(3, 2);

  // Establecer un píxel específico
  Color const test_color(1.0, 0.0, 0.0);  // Rojo
  image.set_pixel(2, test_color);

  // Verificar que otros píxeles siguen siendo negros (0, 0, 0)
  for (size_t i = 0; i < image.height() * image.width(); ++i) {
    if (i == 2) {
      ImageAOS::Pixel const pixel = image.get_pixel(i);
      // El píxel 2 debe ser rojo
      ASSERT_EQ(pixel.r, 255) << "Píxel 2 debe tener R=255";
      ASSERT_EQ(pixel.g, 0) << "Píxel 2 debe tener G=0";
      ASSERT_EQ(pixel.b, 0) << "Píxel 2 debe tener B=0";
    } else {
      // Todos los demás píxeles deben seguir siendo negros
      ImageAOS::Pixel const pixel = image.get_pixel(i);
      ASSERT_EQ(pixel.r, 0) << "Píxel " << i << " componente R debe ser 0";
      ASSERT_EQ(pixel.g, 0) << "Píxel " << i << " componente G debe ser 0";
      ASSERT_EQ(pixel.b, 0) << "Píxel " << i << " componente B debe ser 0";
    }
  }
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
  ImageAOS::Pixel const pixel0 = image.get_pixel(0);
  ASSERT_EQ(pixel0.r, 255);
  ASSERT_EQ(pixel0.g, 0);
  ASSERT_EQ(pixel0.b, 0);

  // Píxel 1: Verde
  ImageAOS::Pixel const pixel1 = image.get_pixel(1);
  ASSERT_EQ(pixel1.r, 0);
  ASSERT_EQ(pixel1.g, 255);
  ASSERT_EQ(pixel1.b, 0);

  // Píxel 2: Azul
  ImageAOS::Pixel const pixel2 = image.get_pixel(2);
  ASSERT_EQ(pixel2.r, 0);
  ASSERT_EQ(pixel2.g, 0);
  ASSERT_EQ(pixel2.b, 255);

  // Píxel 3: Amarillo
  ImageAOS::Pixel const pixel3 = image.get_pixel(3);
  ASSERT_EQ(pixel3.r, 255);
  ASSERT_EQ(pixel3.g, 255);
  ASSERT_EQ(pixel3.b, 0);

  // Píxel 4: Magenta
  ImageAOS::Pixel const pixel4 = image.get_pixel(4);
  ASSERT_EQ(pixel4.r, 255);
  ASSERT_EQ(pixel4.g, 0);
  ASSERT_EQ(pixel4.b, 255);

  // Píxel 5: Cian
  ImageAOS::Pixel const pixel5 = image.get_pixel(5);
  ASSERT_EQ(pixel5.r, 0);
  ASSERT_EQ(pixel5.g, 255);
  ASSERT_EQ(pixel5.b, 255);
}

// ============================================================================
// TESTS PARA fill_from_double
// ============================================================================

// Test 17: fill_from_double con datos válidos
TEST_F(ImageAOSTest, FillFromDoubleValidData) {
  // Configuración: imagen de 1x2 (2 píxeles)
  ImageAOS image(1, 2);

  // Gamma personalizado
  double const gamma = 2.0;

  // Datos de entrada (tamaño = 2)
  std::vector<double> const r_data = {0.25, 0.5};
  std::vector<double> const g_data = {0.5, 0.75};
  std::vector<double> const b_data = {1.0, 0.0};

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
  ImageAOS::Pixel const p0 = image.get_pixel(0);
  ASSERT_EQ(p0.r, expected_r0) << "Píxel 0: componente R incorrecto";
  ASSERT_EQ(p0.g, expected_g0) << "Píxel 0: componente G incorrecto";
  ASSERT_EQ(p0.b, expected_b0) << "Píxel 0: componente B incorrecto (debería ser 255)";

  // Verificar Píxel 1
  ImageAOS::Pixel const p1 = image.get_pixel(1);
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
  std::vector<double> const r_data = {0.5};
  std::vector<double> const g_data = {0.5};
  std::vector<double> const b_data = {0.5};

  // Llamar a fill_from_double SIN especificar gamma (usa el default)
  image.fill_from_double(r_data, g_data, b_data);

  // Calcular valor esperado con gamma por defecto (Constants::Gamma)
  // Asumiendo que el default es 2.2 según constants.hpp
  double const default_gamma = 2.2;  // Constants::Gamma
  auto expected              = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));

  // Verificar píxel
  ImageAOS::Pixel const p = image.get_pixel(0);
  ASSERT_EQ(p.r, expected) << "Componente R con gamma default incorrecto";
  ASSERT_EQ(p.g, expected) << "Componente G con gamma default incorrecto";
  ASSERT_EQ(p.b, expected) << "Componente B con gamma default incorrecto";
}

// Test 21: fill_from_double con valores extremos
TEST_F(ImageAOSTest, FillFromDoubleExtremeValues) {
  // Configuración: imagen de 3x1
  ImageAOS image(3, 1);

  double const gamma = 2.0;

  // Datos con valores extremos: 0.0, 1.0, y un valor intermedio
  std::vector<double> const r_data = {0.0, 1.0, 0.5};
  std::vector<double> const g_data = {1.0, 0.0, 0.5};
  std::vector<double> const b_data = {0.5, 0.5, 1.0};

  // Llamar a fill_from_double
  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Verificar píxel 0: r=0.0 (negro), g=1.0 (blanco), b=0.5
  ImageAOS::Pixel const p0 = image.get_pixel(0);
  ASSERT_EQ(p0.r, 0) << "Valor 0.0 debe resultar en 0";
  ASSERT_EQ(p0.g, 255) << "Valor 1.0 debe resultar en 255";

  // Verificar píxel 1: r=1.0 (blanco), g=0.0 (negro), b=0.5
  ImageAOS::Pixel const p1 = image.get_pixel(1);
  ASSERT_EQ(p1.r, 255) << "Valor 1.0 debe resultar en 255";
  ASSERT_EQ(p1.g, 0) << "Valor 0.0 debe resultar en 0";

  // Verificar píxel 2: todos los valores 0.5 o 1.0
  ImageAOS::Pixel const p2 = image.get_pixel(2);
  auto expected_05         = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
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
  ImageAOS::Pixel const pixel0 = image.get_pixel(0);
  ImageAOS::Pixel const pixel1 = image.get_pixel(1);
  ASSERT_EQ(pixel0.r, 255);
  ASSERT_EQ(pixel1.g, 255);

  // Ahora llamar a fill_from_double con datos diferentes
  std::vector<double> const r_data = {0.0, 0.0};
  std::vector<double> const g_data = {0.0, 0.0};
  std::vector<double> const b_data = {1.0, 1.0};  // Azul

  image.fill_from_double(r_data, g_data, b_data);

  // Verificar que los píxeles fueron sobrescritos
  ImageAOS::Pixel const p0 = image.get_pixel(0);
  ImageAOS::Pixel const p1 = image.get_pixel(1);

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
  size_t const size = 100;
  ImageAOS image(10, 10);

  // Crear datos de entrada (todos con valor 0.5)
  std::vector<double> const r_data(size, 0.5);
  std::vector<double> const g_data(size, 0.5);
  std::vector<double> const b_data(size, 0.5);

  double const gamma = 2.0;

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
    ImageAOS::Pixel const p = image.get_pixel(i);
    ASSERT_EQ(p.r, expected) << "Píxel " << i << " componente R incorrecto";
    ASSERT_EQ(p.g, expected) << "Píxel " << i << " componente G incorrecto";
    ASSERT_EQ(p.b, expected) << "Píxel " << i << " componente B incorrecto";
  }
}

// ============================================================================
// TESTS PARA write_to_ppm
// ============================================================================

// Test 24: write_to_ppm con imagen válida
TEST_F(ImageAOSTest, WriteToPPMValidImage) {
  // Configuración: imagen de 2x1 (2 píxeles)
  ImageAOS image(2, 1);

  // Establecer píxeles con valores conocidos
  // Píxel 0: Rojo (255, 0, 0)
  image.set_pixel(0, Color(1.0, 0.0, 0.0));
  // Píxel 1: Azul (0, 0, 255)
  image.set_pixel(1, Color(0.0, 0.0, 1.0));

  // Definir nombre de archivo temporal
  std::string const filename = "test_aos.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió true
  ASSERT_TRUE(result) << "write_to_ppm debe devolver true para imagen válida";

  // Verificar el contenido del archivo
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo " << filename << " debe existir y ser legible";

  // Leer y verificar la cabecera PPM
  std::string line;

  // Línea 1: "P3"
  std::getline(file, line);
  ASSERT_EQ(line, "P3") << "Primera línea debe ser 'P3' (formato PPM ASCII)";

  // Línea 2: Dimensiones "2 1"
  std::getline(file, line);
  ASSERT_EQ(line, "2 1") << "Segunda línea debe contener dimensiones '2 1'";

  // Línea 3: Valor máximo "255"
  std::getline(file, line);
  ASSERT_EQ(line, "255") << "Tercera línea debe ser '255' (valor máximo de color)";

  // Leer píxeles
  // Píxel 0: Rojo (255, 0, 0)
  int r0 = 0;
  int g0 = 0;
  int b0 = 0;
  file >> r0 >> g0 >> b0;
  ASSERT_EQ(r0, 255) << "Píxel 0: componente R debe ser 255 (rojo)";
  ASSERT_EQ(g0, 0) << "Píxel 0: componente G debe ser 0";
  ASSERT_EQ(b0, 0) << "Píxel 0: componente B debe ser 0";

  // Píxel 1: Azul (0, 0, 255)
  int r1 = 0;
  int g1 = 0;
  int b1 = 0;
  file >> r1 >> g1 >> b1;
  ASSERT_EQ(r1, 0) << "Píxel 1: componente R debe ser 0";
  ASSERT_EQ(g1, 0) << "Píxel 1: componente G debe ser 0";
  ASSERT_EQ(b1, 255) << "Píxel 1: componente B debe ser 255 (azul)";

  // Cerrar el archivo
  file.close();

  // Nota: La limpieza del archivo se hace en TearDown()
}

// Test 25: write_to_ppm con ruta inválida
TEST_F(ImageAOSTest, WriteToPPMInvalidPath) {
  // Configuración: imagen de 1x1
  ImageAOS image(1, 1);

  // Establecer píxel con valor conocido
  image.set_pixel(0, Color(0.0039215, 0.0078431, 0.0117647));  // (1, 2, 3) en uint8_t

  // Intentar escribir a un directorio que no existe
  std::string const filename = "invalid_dir/test_error_aos.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió false (propagando el error de PPMWriter)
  ASSERT_FALSE(result) << "write_to_ppm debe devolver false cuando la ruta es inválida";
}

// Test 26: write_to_ppm con imagen compleja (varios píxeles)
TEST_F(ImageAOSTest, WriteToPPMComplexImage) {
  // Configuración: imagen de 3x2 (6 píxeles)
  ImageAOS image(3, 2);

  // Establecer píxeles con diferentes colores
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde
  image.set_pixel(2, Color(0.0, 0.0, 1.0));  // Azul
  image.set_pixel(3, Color(1.0, 1.0, 0.0));  // Amarillo
  image.set_pixel(4, Color(1.0, 0.0, 1.0));  // Magenta
  image.set_pixel(5, Color(0.0, 1.0, 1.0));  // Cian

  // Definir nombre de archivo temporal
  std::string const filename = "test_aos.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió true
  ASSERT_TRUE(result) << "write_to_ppm debe devolver true para imagen válida";

  // Verificar el contenido del archivo
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo debe existir y ser legible";

  // Leer y verificar la cabecera PPM
  std::string line;

  // Línea 1: "P3"
  std::getline(file, line);
  ASSERT_EQ(line, "P3");

  // Línea 2: Dimensiones "3 2"
  std::getline(file, line);
  ASSERT_EQ(line, "3 2") << "Dimensiones deben ser '3 2'";

  // Línea 3: Valor máximo "255"
  std::getline(file, line);
  ASSERT_EQ(line, "255");

  // Leer y verificar los 6 píxeles
  std::vector<std::tuple<int, int, int>> expected_pixels = {
    {255,   0,   0}, // Rojo
    {  0, 255,   0}, // Verde
    {  0,   0, 255}, // Azul
    {255, 255,   0}, // Amarillo
    {255,   0, 255}, // Magenta
    {  0, 255, 255}  // Cian
  };

  for (size_t i = 0; i < expected_pixels.size(); ++i) {
    int r = 0;
    int g = 0;
    int b = 0;
    file >> r >> g >> b;

    auto [exp_r, exp_g, exp_b] = expected_pixels[i];
    ASSERT_EQ(r, exp_r) << "Píxel " << i << ": componente R incorrecto";
    ASSERT_EQ(g, exp_g) << "Píxel " << i << ": componente G incorrecto";
    ASSERT_EQ(b, exp_b) << "Píxel " << i << ": componente B incorrecto";
  }

  // Cerrar el archivo
  file.close();
}

// Test 27: write_to_ppm preserva el estado interno de la imagen
TEST_F(ImageAOSTest, WriteToPPMPreservesImageState) {
  // Configuración: imagen de 2x1
  ImageAOS image(2, 1);

  // Establecer píxeles
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde

  // Guardar valores antes de write_to_ppm
  ImageAOS::Pixel const p0_before = image.get_pixel(0);
  ImageAOS::Pixel const p1_before = image.get_pixel(1);

  // Definir nombre de archivo temporal
  std::string const filename = "test_aos.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);
  ASSERT_TRUE(result);

  // Verificar que los píxeles NO cambiaron después de write_to_ppm
  ImageAOS::Pixel const p0_after = image.get_pixel(0);
  ImageAOS::Pixel const p1_after = image.get_pixel(1);

  ASSERT_EQ(p0_after.r, p0_before.r) << "Píxel 0 R no debe cambiar después de write_to_ppm";
  ASSERT_EQ(p0_after.g, p0_before.g) << "Píxel 0 G no debe cambiar después de write_to_ppm";
  ASSERT_EQ(p0_after.b, p0_before.b) << "Píxel 0 B no debe cambiar después de write_to_ppm";

  ASSERT_EQ(p1_after.r, p1_before.r) << "Píxel 1 R no debe cambiar después de write_to_ppm";
  ASSERT_EQ(p1_after.g, p1_before.g) << "Píxel 1 G no debe cambiar después de write_to_ppm";
  ASSERT_EQ(p1_after.b, p1_before.b) << "Píxel 1 B no debe cambiar después de write_to_ppm";
}

// Test 28: write_to_ppm múltiples veces al mismo archivo
TEST_F(ImageAOSTest, WriteToPPMMultipleTimes) {
  // Configuración: imagen de 1x1
  ImageAOS image(1, 1);

  // Primera escritura: píxel rojo
  image.set_pixel(0, Color(1.0, 0.0, 0.0));

  std::string const filename = "test_aos.ppm";

  // Primera llamada a write_to_ppm
  bool const result1 = image.write_to_ppm(filename);
  ASSERT_TRUE(result1);

  // Modificar la imagen: píxel verde
  image.set_pixel(0, Color(0.0, 1.0, 0.0));

  // Segunda llamada a write_to_ppm (sobrescribe el archivo)
  bool const result2 = image.write_to_ppm(filename);
  ASSERT_TRUE(result2);

  // Verificar que el archivo contiene el píxel VERDE (segunda escritura)
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open());

  std::string line;
  // Saltar cabecera
  std::getline(file, line);  // P3
  std::getline(file, line);  // 1 1
  std::getline(file, line);  // 255

  // Leer píxel
  int r = 0;
  int g = 0;
  int b = 0;
  file >> r >> g >> b;

  ASSERT_EQ(r, 0) << "Píxel R debe ser 0 (verde)";
  ASSERT_EQ(g, 255) << "Píxel G debe ser 255 (verde)";
  ASSERT_EQ(b, 0) << "Píxel B debe ser 0 (verde)";

  file.close();
}
