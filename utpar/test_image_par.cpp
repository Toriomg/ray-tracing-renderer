#include "../par/include/image_par.hpp"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// Tests para verificar que los arrays se generan del tamaño correcto
TEST(test_image_par, numero_pixeles) {
  ImagePar const img(10, 10);
  EXPECT_EQ(img.total_pixels(), 10 * 10);
}

TEST(test_image_par, calculo_indice) {
  ImagePar const img(10, 10);
  EXPECT_EQ(img.indice(0, 0), 0);
  EXPECT_EQ(img.indice(0, 1), 1);
  EXPECT_EQ(img.indice(1, 0), 10);
  EXPECT_EQ(img.indice(1, 1), 11);
}

// ============================================================================
// TESTS PARA fill_from_double
// ============================================================================

// Test: fill_from_double con datos válidos
TEST(test_image_par, fill_from_double_valid_data) {
  ImagePar image(1, 2);  // 2 píxeles
  double const gamma               = 2.0;
  std::vector<double> const r_data = {0.25, 0.5};
  std::vector<double> const g_data = {0.5, 0.75};
  std::vector<double> const b_data = {1.0, 0.0};

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

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
TEST(test_image_par, fill_from_double_default_gamma) {
  ImagePar image(1, 1);
  std::vector<double> const r = {0.5};
  std::vector<double> const g = {0.5};
  std::vector<double> const b = {0.5};
  RGBInputData input{&r, &g, &b};
  image.fill_from_double(input, Constants::Gamma, nullptr);  // Usando gamma por defecto

  // Gamma por defecto es Constants::Gamma (asumimos 2.2)
  double const default_gamma = 2.2;
  auto expected              = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_green(0), expected);
  EXPECT_EQ(image.get_blue(0), expected);
}

// Test: fill_from_double con valores extremos
TEST(test_image_par, fill_from_double_extreme_values) {
  ImagePar image(3, 1);  // 3 píxeles
  double const gamma = 2.0;

  std::vector<double> const r_data = {0.0, 1.0, 0.5};
  std::vector<double> const g_data = {1.0, 0.0, 0.5};
  std::vector<double> const b_data = {0.5, 0.5, 1.0};

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

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
TEST(test_image_par, fill_from_double_overwrites_previous_data) {
  ImagePar image(2, 1);  // 2 píxeles

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

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, Constants::Gamma, nullptr);

  // Verificar que los píxeles fueron sobrescritos
  EXPECT_EQ(image.get_red(0), 0) << "Píxel 0 R debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_green(0), 0) << "Píxel 0 G debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_blue(0), 255) << "Píxel 0 B debe haber sido sobrescrito a 255";

  EXPECT_EQ(image.get_red(1), 0) << "Píxel 1 R debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_green(1), 0) << "Píxel 1 G debe haber sido sobrescrito a 0";
  EXPECT_EQ(image.get_blue(1), 255) << "Píxel 1 B debe haber sido sobrescrito a 255";
}

// Test: fill_from_double con imagen grande
TEST(test_image_par, fill_from_double_large_image) {
  size_t const size = 100;
  ImagePar image(10, 10);  // 100 píxeles

  std::vector<double> const r_data(size, 0.5);
  std::vector<double> const g_data(size, 0.5);
  std::vector<double> const b_data(size, 0.5);

  double const gamma = 2.0;
  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

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

class ImageParIOTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización si es necesaria
  }

  void TearDown() override {
    // Limpieza de archivos temporales creados durante los tests
    static_cast<void>(std::remove("test_par.ppm"));
    static_cast<void>(std::remove("test_error_par.ppm"));
  }
};

// ============================================================================
// TESTS PARA write_to_ppm
// ============================================================================

// Test: write_to_ppm con imagen válida
TEST_F(ImageParIOTest, WriteToPPMValidImage) {
  // Configuración: imagen de 2x1 (2 píxeles)
  ImagePar image(2, 1);

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
  std::string const filename = "test_par.ppm";

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
TEST_F(ImageParIOTest, WriteToPPMInvalidPath) {
  // Configuración: imagen de 1x1
  ImagePar image(1, 1);

  // Establecer píxel con valores conocidos
  image.set_red(0, 0.0039215);    // ~1 en uint8_t
  image.set_green(0, 0.0078431);  // ~2 en uint8_t
  image.set_blue(0, 0.0117647);   // ~3 en uint8_t

  // Intentar escribir a un directorio que no existe
  std::string const filename = "invalid_dir/test_error_par.ppm";

  // Llamar a write_to_ppm
  bool const result = image.write_to_ppm(filename);

  // Verificar que la función devolvió false (propagando el error de PPMWriter)
  EXPECT_FALSE(result) << "write_to_ppm debe devolver false cuando la ruta es inválida";
}

// Test: write_to_ppm con imagen compleja (varios píxeles)
namespace {

  // Estructura Auxiliar para representar el color de un píxel en formato 0-255
  struct ExpectedPixel {
    int r, g, b;
  };

  // Función Auxiliar para Verificar la Cabecera de un archivo PPM
  void verifyPPMHeader(std::ifstream & file, int expected_width, int expected_height) {
    std::string line;
    ASSERT_TRUE(std::getline(file, line));
    EXPECT_EQ(line, "P3");

    ASSERT_TRUE(std::getline(file, line));
    EXPECT_EQ(line, std::to_string(expected_width) + " " + std::to_string(expected_height));

    ASSERT_TRUE(std::getline(file, line));
    EXPECT_EQ(line, "255");
  }

  // Función Auxiliar para Verificar un Píxel Específico
  void verifyPixel(std::ifstream & file, size_t pixel_index, ExpectedPixel const & expected_color) {
    int r = 0, g = 0, b = 0;
    // Ahora los tipos coinciden, la advertencia desaparece.
    ASSERT_TRUE(file >> r >> g >> b) << "Fallo al leer los datos del píxel " << pixel_index;

    EXPECT_EQ(r, expected_color.r) << "Píxel " << pixel_index << ": componente R incorrecto";
    EXPECT_EQ(g, expected_color.g) << "Píxel " << pixel_index << ": componente G incorrecto";
    EXPECT_EQ(b, expected_color.b) << "Píxel " << pixel_index << ": componente B incorrecto";
  }

}  // namespace

// --- PRUEBA PRINCIPAL (AHORA SIMPLIFICADA) ---
TEST_F(ImageParIOTest, WriteToPPMComplexImage) {
  // 1. Arrange (Preparar)
  ImagePar image(3, 2);
  image.set_pixel(0, Color(1.0, 0.0, 0.0));
  image.set_pixel(1, Color(0.0, 1.0, 0.0));
  image.set_pixel(2, Color(0.0, 0.0, 1.0));
  image.set_pixel(3, Color(1.0, 1.0, 0.0));
  image.set_pixel(4, Color(1.0, 0.0, 1.0));
  image.set_pixel(5, Color(0.0, 1.0, 1.0));

  std::string const filename = "test_par.ppm";

  // 2. Act (Actuar)
  bool const result = image.write_to_ppm(filename);
  EXPECT_TRUE(result) << "write_to_ppm debe devolver true para una operación exitosa";

  // 3. Assert (Verificar)
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo PPM no se pudo abrir para verificación";

  verifyPPMHeader(file, 3, 2);

  // Los datos esperados ahora son mucho más explícitos y fáciles de leer.
  std::vector<ExpectedPixel> const expected_pixels = {
    {255,   0,   0}, // Rojo
    {  0, 255,   0}, // Verde
    {  0,   0, 255}, // Azul
    {255, 255,   0}, // Amarillo
    {255,   0, 255}, // Magenta
    {  0, 255, 255}  // Cian
  };

  // El bucle de verificación ahora es simple y no tiene condicionales.
  for (size_t i = 0; i < expected_pixels.size(); ++i) {
    verifyPixel(file, i, expected_pixels[i]);
  }

  // Verificar que no hay más datos en el archivo
  int dummy = 0;
  EXPECT_FALSE(file >> dummy) << "Hay datos extra en el archivo después de los píxeles";

  file.close();
}

// Test: write_to_ppm preserva el estado interno de la imagen
TEST_F(ImageParIOTest, WriteToPPMPreservesImageState) {
  // Configuración: imagen de 2x1
  ImagePar image(2, 1);

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
  std::string const filename = "test_par.ppm";

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
TEST_F(ImageParIOTest, WriteToPPMMultipleTimes) {
  // Configuración: imagen de 1x1
  ImagePar image(1, 1);

  // Primera escritura: píxel rojo
  image.set_pixel(0, Color(1.0, 0.0, 0.0));

  std::string const filename = "test_par.ppm";

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
