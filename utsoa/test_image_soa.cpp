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
