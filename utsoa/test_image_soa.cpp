#include "../soa/include/image_soa.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

// Tests para verificar que los arrays se generan del tamaño correcto
TEST(test_image_soa, numero_pixeles) {
  ImageSOA img(10, 10);
  EXPECT_EQ(img.total_pixels(), 10 * 10);
}

TEST(test_image_soa, dimension_cero) {
  EXPECT_THROW(ImageSOA img(0, 1), std::invalid_argument);
  EXPECT_THROW(ImageSOA img(5, 0), std::invalid_argument);
  EXPECT_THROW(ImageSOA img(0, 0), std::invalid_argument);
}

TEST(test_image_soa, longitud_colores) {
  ImageSOA img(50, 5);
  EXPECT_EQ(img.get_r_channel().size(), 50 * 5);
  EXPECT_EQ(img.get_g_channel().size(), 50 * 5);
  EXPECT_EQ(img.get_b_channel().size(), 50 * 5);
}

TEST(test_image_soa, calculo_indice) {
  ImageSOA img(10, 10);
  EXPECT_EQ(img.indice(0, 0), 0);
  EXPECT_EQ(img.indice(0, 1), 1);
  EXPECT_EQ(img.indice(1, 0), 10);
  EXPECT_EQ(img.indice(1, 1), 11);
}

TEST(test_image_soa, indice_fuera_rango) {
  ImageSOA img(10, 10);
  EXPECT_THROW(static_cast<void>(img.indice(10, 0)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.indice(0, 10)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.indice(11, 11)), std::out_of_range);
}

// Tests para comprobar que si se introduce un pixel fuera de rango las funciones fallan
TEST(test_image_soa, get_fuera_rango) {
  ImageSOA img(10, 10);
  EXPECT_THROW(static_cast<void>(img.get_red(100)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.get_green(101)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.get_blue(150)), std::out_of_range);
}

TEST(test_image_soa, set_fuera_rango) {
  ImageSOA img(10, 10);
  EXPECT_THROW(static_cast<void>(img.set_red(100, 67)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_green(100, 101)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_blue(100, 150)), std::out_of_range);
}

TEST(test_image_soa, color_invalido) {
  ImageSOA img(10, 10);
  EXPECT_THROW(static_cast<void>(img.set_red(0, -5.0F)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_red(0, 300.0F)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_green(0, -1.0F)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_green(0, 256.0F)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_blue(0, -0.1F)), std::out_of_range);
  EXPECT_THROW(static_cast<void>(img.set_blue(0, 500.0F)), std::out_of_range);
}

// ============================================================================
// TESTS PARA fill_from_double
// ============================================================================

// Test: fill_from_double con datos válidos
TEST(test_image_soa, fill_from_double_valid_data) {
  ImageSOA image(1, 2);  // 2 píxeles
  double gamma               = 2.0;
  std::vector<double> r_data = {0.25, 0.5};
  std::vector<double> g_data = {0.5, 0.75};
  std::vector<double> b_data = {1.0, 0.0};

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
  std::vector<double> r = {0.5};
  std::vector<double> g = {0.5};
  std::vector<double> b = {0.5};
  image.fill_from_double(r, g, b);  // SIN especificar gamma (usa Constants::Gamma)

  // Gamma por defecto es Constants::Gamma (asumimos 2.2)
  double default_gamma = 2.2;
  auto expected        = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_green(0), expected);
  EXPECT_EQ(image.get_blue(0), expected);
}

// Test: fill_from_double lanza excepción con tamaño inválido (canal R)
TEST(test_image_soa, fill_from_double_throws_invalid_size_r) {
  ImageSOA image(3, 3);                // 9 píxeles
  std::vector<double> r_data = {0.5};  // Tamaño incorrecto (1 en lugar de 9)
  std::vector<double> g_data(9, 0.5);
  std::vector<double> b_data(9, 0.5);
  EXPECT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument);
}

// Test: fill_from_double lanza excepción con tamaño inválido (canal G)
TEST(test_image_soa, fill_from_double_throws_invalid_size_g) {
  ImageSOA image(3, 3);
  std::vector<double> r_data(9, 0.5);
  std::vector<double> g_data = {0.5};  // Tamaño incorrecto
  std::vector<double> b_data(9, 0.5);
  EXPECT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument);
}

// Test: fill_from_double lanza excepción con tamaño inválido (canal B)
TEST(test_image_soa, fill_from_double_throws_invalid_size_b) {
  ImageSOA image(3, 3);
  std::vector<double> r_data(9, 0.5);
  std::vector<double> g_data(9, 0.5);
  std::vector<double> b_data = {0.5};  // Tamaño incorrecto
  EXPECT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument);
}

// Test: fill_from_double con todos los canales de tamaño incorrecto
TEST(test_image_soa, fill_from_double_throws_all_channels_wrong_size) {
  ImageSOA image(2, 2);  // 4 píxeles esperados
  std::vector<double> r_data = {0.5};
  std::vector<double> g_data = {0.5};
  std::vector<double> b_data = {0.5};
  EXPECT_THROW(image.fill_from_double(r_data, g_data, b_data), std::invalid_argument);
}

// Test: fill_from_double con valores extremos
TEST(test_image_soa, fill_from_double_extreme_values) {
  ImageSOA image(3, 1);  // 3 píxeles
  double gamma = 2.0;

  std::vector<double> r_data = {0.0, 1.0, 0.5};
  std::vector<double> g_data = {1.0, 0.0, 0.5};
  std::vector<double> b_data = {0.5, 0.5, 1.0};

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
  std::vector<double> r_data = {0.0, 0.0};
  std::vector<double> g_data = {0.0, 0.0};
  std::vector<double> b_data = {1.0, 1.0};  // Azul

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
  size_t size = 100;
  ImageSOA image(10, 10);  // 100 píxeles

  std::vector<double> r_data(size, 0.5);
  std::vector<double> g_data(size, 0.5);
  std::vector<double> b_data(size, 0.5);

  double gamma = 2.0;
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
