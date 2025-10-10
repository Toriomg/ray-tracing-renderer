#include "../soa/include/image_soa.hpp"
#include <gtest/gtest.h>

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
