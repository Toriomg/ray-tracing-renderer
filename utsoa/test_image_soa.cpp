#include "../soa/include/image_soa.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include <vector>

// Tests para verificar que los arrays se generan del tamaño correcto
TEST(test_image_soa, numero_pixeles) {
  ImageSOA const img(10, 10);
  EXPECT_EQ(img.total_pixels(), 10 * 10);
}

TEST(test_image_soa, calculo_indice) {
  ImageSOA const img(10, 10);
  EXPECT_EQ(img.indice(0, 0), 0);
  EXPECT_EQ(img.indice(0, 1), 1);
  EXPECT_EQ(img.indice(1, 0), 10);
  EXPECT_EQ(img.indice(1, 1), 11);
}

// ============================================================================
// TESTS PARA fill_from_double
// ============================================================================

// Test: fill_from_double con datos válidos
TEST(test_image_soa, fill_from_double_valid_data) {
  ImageSOA image(1, 2);  // 2 píxeles
  double const gamma               = 2.0;
  std::vector<double> const r_data = {0.25, 0.5};
  std::vector<double> const g_data = {0.5, 0.75};
  std::vector<double> const b_data = {1.0, 0.0};

  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Verifica Píxel 0
  auto expected_r0 = static_cast<uint8_t>(255.999 * std::pow(0.25, 1.0 / gamma));
  auto expected_g0 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_b0 = static_cast<uint8_t>(255.999 * std::pow(1.0, 1.0 / gamma));  // 255
  EXPECT_EQ(image.get_red(0), expected_r0);
  EXPECT_EQ(image.get_green(0), expected_g0);
  EXPECT_EQ(image.get_blue(0), expected_b0);
  EXPECT_EQ(image.get_blue(0), 255) << "Valor 1.0 con gamma debe resultar en 255";

  // Verifica Píxel 1
  auto expected_r1 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_g1 = static_cast<uint8_t>(255.999 * std::pow(0.75, 1.0 / gamma));
  auto expected_b1 = static_cast<uint8_t>(255.999 * std::pow(0.0, 1.0 / gamma));  // 0
  EXPECT_EQ(image.get_red(1), expected_r1);
  EXPECT_EQ(image.get_green(1), expected_g1);
  EXPECT_EQ(image.get_blue(1), expected_b1);
  EXPECT_EQ(image.get_blue(1), 0) << "Valor 0.0 con gamma debe resultar en 0";
}

// Test: fill_from_double con gamma por defecto
TEST(test_image_soa, fill_from_double_default_gamma) {
  ImageSOA image(1, 1);
  std::vector<double> const r = {0.5};
  std::vector<double> const g = {0.5};
  std::vector<double> const b = {0.5};
  image.fill_from_double(r, g, b);  // SIN especificar gamma (usa Constants::Gamma)

  // Gamma por defecto es Constants::Gamma (asumimos 2.2)
  double const default_gamma = 2.2;
  auto expected              = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_green(0), expected);
  EXPECT_EQ(image.get_blue(0), expected);
}

// Test: fill_from_double con valores extremos
TEST(test_image_soa, fill_from_double_extreme_values) {
  ImageSOA image(3, 1);  // 3 píxeles
  double const gamma = 2.0;

  std::vector<double> const r_data = {0.0, 1.0, 0.5};
  std::vector<double> const g_data = {1.0, 0.0, 0.5};
  std::vector<double> const b_data = {0.5, 0.5, 1.0};

  image.fill_from_double(r_data, g_data, b_data, gamma);

  // Píxel 0: r=0.0 (negro), g=1.0 (blanco), b=0.5
  EXPECT_EQ(image.get_red(0), 0) << "Valor 0.0 debe resultar en 0";
  EXPECT_EQ(image.get_green(0), 255) << "Valor 1.0 debe resultar en 255";

  // Píxel 1: r=1.0 (blanco), g=0.0 (negro), b=0.5
  EXPECT_EQ(image.get_red(1), 255) << "Valor 1.0 debe resultar en 255";
  EXPECT_EQ(image.get_green(1), 0) << "Valor 0.0 debe resultar en 0";

  // Píxel 2: b=1.0 (blanco)
  EXPECT_EQ(image.get_blue(2), 255) << "Valor 1.0 debe resultar en 255";
}

// Test: fill_from_double sobrescribe datos anteriores
TEST(test_image_soa, fill_from_double_overwrites_previous_data) {
  ImageSOA image(2, 1);  // 2 píxeles

  // Establecer píxeles manualmente
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde

  // Verificar que se establecieron
  EXPECT_EQ(image.get_red(0), 255);
  EXPECT_EQ(image.get_green(1), 255);

  // Llamar a fill_from_double con datos diferentes
  std::vector<double> const r_data = {0.0, 0.0};
  std::vector<double> const g_data = {0.0, 0.0};
  std::vector<double> const b_data = {1.0, 1.0};  // Azul

  image.fill_from_double(r_data, g_data, b_data);

  // Verificar que los píxeles fueron sobrescritos
  EXPECT_EQ(image.get_red(0), 0) << "Píxel 0 R debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_green(0), 0) << "Píxel 0 G debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_blue(0), 255) << "Píxel 0 B debe haber sido sobrescrito a 255";

  EXPECT_EQ(image.get_red(1), 0) << "Píxel 1 R debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_green(1), 0) << "Píxel 1 G debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_blue(1), 255) << "Píxel 1 B debe haber sido sobrescrito a 255";
}

// Test: fill_from_double con imagen grande
TEST(test_image_soa, fill_from_double_large_image) {
  size_t const size = 100;
  ImageSOA image(10, 10);  // 100 píxeles

  std::vector<double> const r_data(size, 0.5);
  std::vector<double> const g_data(size, 0.5);
  std::vector<double> const b_data(size, 0.5);

  double const gamma = 2.0;
  image.fill_from_double(r_data, g_data, b_data, gamma);

  auto expected = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));

  // Verificar algunos píxeles aleatorios
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_green(50), expected);
  EXPECT_EQ(image.get_blue(99), expected);

  // Verificar que todos los píxeles tienen el mismo valor
  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(image.get_red(i), expected) << "Píxel " << i << " componente R incorrecto";
    EXPECT_EQ(image.get_green(i), expected) << "Píxel " << i << " componente G incorrecto";
    EXPECT_EQ(image.get_blue(i), expected) << "Píxel " << i << " componente B incorrecto";
  }
}

// ============================================================================
// FIXTURE PARA write_to_ppm
// ============================================================================

class ImageSOAIOTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización si es necesaria
  }

  void TearDown() override {
    // Limpieza de archivos temporales creados durante los tests
    static_cast<void>(std::remove("test_soa.ppm"));
    static_cast<void>(std::remove("test_error_soa.ppm"));
  }
};

// ============================================================================
// TESTS PARA write_to_ppm
// ============================================================================

// Test: write_to_ppm con imagen válida
TEST_F(ImageSOAIOTest, WriteToPPMValidImage) {
  // Configuración: imagen de 2x1 (2 píxeles)
  ImageSOA image(2, 1);

  // Establecer píxeles con valores conocidos usando setters individuales
  // Píxel 0: Rojo (255, 0, 0)
  image.set_red(0, 1.0);
  image.set_green(0, 0.0);
  image.set_blue(0, 0.0);

  // Píxel 1: Verde medio (0, 128, 0)
  // Para obtener 128, necesitamos un valor que con gamma 2.2 dé ~128
  // Aproximadamente: 128/255 = 0.502, entonces pow(x, 1/2.2) = 0.502 => x ≈ 0.216
  // Usamos un valor directo para simplificar: establecemos aproximadamente 0.5 que dará ~186
  // Mejor usar un valor calculado inverso o establecer directamente
  image.set_red(1, 0.0);
  image.set_green(1, 0.216);  // Este valor con gamma 2.2 debería dar ~128
  image.set_blue(1, 0.0);

  // Definir nombre de archivo temporal
  std::string const filename = "test_soa.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió true
  EXPECT_TRUE(result) << "write_to_ppm debe devolver true para imagen válida";

  // Verificar el contenido del archivo
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo " << filename << " debe existir y ser legible";

  // Leer y verificar la cabecera PPM
  std::string line;

  // Línea 1: "P3"
  std::getline(file, line);
  EXPECT_EQ(line, "P3") << "Primera línea debe ser 'P3' (formato PPM ASCII)";

  // Línea 2: Dimensiones "2 1"
  std::getline(file, line);
  EXPECT_EQ(line, "2 1") << "Segunda línea debe contener dimensiones '2 1'";

  // Línea 3: Valor máximo "255"
  std::getline(file, line);
  EXPECT_EQ(line, "255") << "Tercera línea debe ser '255' (valor máximo de color)";

  // Leer píxeles
  // Píxel 0: Rojo (255, 0, 0)
  int r0 = 0;
  int g0 = 0;
  int b0 = 0;
  file >> r0 >> g0 >> b0;
  EXPECT_EQ(r0, 255) << "Píxel 0: componente R debe ser 255 (rojo)";
  EXPECT_EQ(g0, 0) << "Píxel 0: componente G debe ser 0";
  EXPECT_EQ(b0, 0) << "Píxel 0: componente B debe ser 0";

  // Píxel 1: Verde (valor depende del gamma, verificamos que G > 0 y R,B = 0)
  int r1 = 0;
  int g1 = 0;
  int b1 = 0;
  file >> r1 >> g1 >> b1;
  EXPECT_EQ(r1, 0) << "Píxel 1: componente R debe ser 0";
  EXPECT_GT(g1, 0) << "Píxel 1: componente G debe ser mayor que 0";
  EXPECT_LT(g1, 256) << "Píxel 1: componente G debe ser menor que 256";
  EXPECT_EQ(b1, 0) << "Píxel 1: componente B debe ser 0";

  // Cerrar el archivo
  file.close();
}

// Test: write_to_ppm con ruta inválida
TEST_F(ImageSOAIOTest, WriteToPPMInvalidPath) {
  // Configuración: imagen de 1x1
  ImageSOA image(1, 1);

  // Establecer píxel con valores conocidos
  image.set_red(0, 0.0039215);    // ~1 en uint8_t
  image.set_green(0, 0.0078431);  // ~2 en uint8_t
  image.set_blue(0, 0.0117647);   // ~3 en uint8_t

  // Intentar escribir a un directorio que no existe
  std::string const filename = "invalid_dir/test_error_soa.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió false (propagando el error de PPMWriter)
  EXPECT_FALSE(result) << "write_to_ppm debe devolver false cuando la ruta es inválida";
}

// Test: write_to_ppm con imagen compleja (varios píxeles)
TEST_F(ImageSOAIOTest, WriteToPPMComplexImage) {
  // Configuración: imagen de 3x2 (6 píxeles)
  ImageSOA image(3, 2);

  // Establecer píxeles con diferentes colores (usando valores 0.0 y 1.0 para simplicidad)
  // Fila 0: Rojo, Verde, Azul
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde
  image.set_pixel(2, Color(0.0, 0.0, 1.0));  // Azul

  // Fila 1: Amarillo, Magenta, Cian
  image.set_pixel(3, Color(1.0, 1.0, 0.0));  // Amarillo
  image.set_pixel(4, Color(1.0, 0.0, 1.0));  // Magenta
  image.set_pixel(5, Color(0.0, 1.0, 1.0));  // Cian

  // Definir nombre de archivo temporal
  std::string const filename = "test_soa.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió true
  EXPECT_TRUE(result) << "write_to_ppm debe devolver true para imagen válida";

  // Verificar el contenido del archivo
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo debe existir y ser legible";

  // Leer y verificar la cabecera PPM
  std::string line;

  // Línea 1: "P3"
  std::getline(file, line);
  EXPECT_EQ(line, "P3");

  // Línea 2: Dimensiones "3 2"
  std::getline(file, line);
  EXPECT_EQ(line, "3 2") << "Dimensiones deben ser '3 2'";

  // Línea 3: Valor máximo "255"
  std::getline(file, line);
  EXPECT_EQ(line, "255");

  // Leer y verificar los 6 píxeles (solo verificamos valores extremos 0 y 255)
  std::vector<std::tuple<bool, bool, bool>> expected_channels = {
    { true, false, false}, // Rojo: R=255, G=0, B=0
    {false,  true, false}, // Verde: R=0, G=255, B=0
    {false, false,  true}, // Azul: R=0, G=0, B=255
    { true,  true, false}, // Amarillo: R=255, G=255, B=0
    { true, false,  true}, // Magenta: R=255, G=0, B=255
    {false,  true,  true}  // Cian: R=0, G=255, B=255
  };

  for (size_t i = 0; i < expected_channels.size(); ++i) {
    int r = 0;
    int g = 0;
    int b = 0;
    file >> r >> g >> b;

    auto [expect_r_max, expect_g_max, expect_b_max] = expected_channels[i];

    if (expect_r_max) {
      EXPECT_EQ(r, 255) << "Píxel " << i << ": componente R debe ser 255";
    } else {
      EXPECT_EQ(r, 0) << "Píxel " << i << ": componente R debe ser 0";
    }

    if (expect_g_max) {
      EXPECT_EQ(g, 255) << "Píxel " << i << ": componente G debe ser 255";
    } else {
      EXPECT_EQ(g, 0) << "Píxel " << i << ": componente G debe ser 0";
    }

    if (expect_b_max) {
      EXPECT_EQ(b, 255) << "Píxel " << i << ": componente B debe ser 255";
    } else {
      EXPECT_EQ(b, 0) << "Píxel " << i << ": componente B debe ser 0";
    }
  }

  // Cerrar el archivo
  file.close();
}

// Test: write_to_ppm preserva el estado interno de la imagen
TEST_F(ImageSOAIOTest, WriteToPPMPreservesImageState) {
  // Configuración: imagen de 2x1
  ImageSOA image(2, 1);

  // Establecer píxeles
  image.set_pixel(0, Color(1.0, 0.0, 0.0));  // Rojo
  image.set_pixel(1, Color(0.0, 1.0, 0.0));  // Verde

  // Guardar valores antes de write_to_ppm
  uint8_t const r0_before = image.get_red(0);
  uint8_t const g0_before = image.get_green(0);
  uint8_t const b0_before = image.get_blue(0);
  uint8_t const r1_before = image.get_red(1);
  uint8_t const g1_before = image.get_green(1);
  uint8_t const b1_before = image.get_blue(1);

  // Definir nombre de archivo temporal
  std::string const filename = "test_soa.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);
  EXPECT_TRUE(result);

  // Verificar que los píxeles NO cambiaron después de write_to_ppm
  EXPECT_EQ(image.get_red(0), r0_before) << "Píxel 0 R no debe cambiar";
  EXPECT_EQ(image.get_green(0), g0_before) << "Píxel 0 G no debe cambiar";
  EXPECT_EQ(image.get_blue(0), b0_before) << "Píxel 0 B no debe cambiar";
  EXPECT_EQ(image.get_red(1), r1_before) << "Píxel 1 R no debe cambiar";
  EXPECT_EQ(image.get_green(1), g1_before) << "Píxel 1 G no debe cambiar";
  EXPECT_EQ(image.get_blue(1), b1_before) << "Píxel 1 B no debe cambiar";
}

// Test: write_to_ppm múltiples veces al mismo archivo
TEST_F(ImageSOAIOTest, WriteToPPMMultipleTimes) {
  // Configuración: imagen de 1x1
  ImageSOA image(1, 1);

  // Primera escritura: píxel rojo
  image.set_pixel(0, Color(1.0, 0.0, 0.0));

  std::string const filename = "test_soa.ppm";

  // Primera llamada a write_to_ppm
  bool const result1 = image.write_to_ppm(filename);
  EXPECT_TRUE(result1);

  // Modificar la imagen: píxel verde
  image.set_pixel(0, Color(0.0, 1.0, 0.0));

  // Segunda llamada a write_to_ppm (sobrescribe el archivo)
  bool const result2 = image.write_to_ppm(filename);
  EXPECT_TRUE(result2);

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

  EXPECT_EQ(r, 0) << "Píxel R debe ser 0 (verde)";
  EXPECT_EQ(g, 255) << "Píxel G debe ser 255 (verde)";
  EXPECT_EQ(b, 0) << "Píxel B debe ser 0 (verde)";

  file.close();
}
