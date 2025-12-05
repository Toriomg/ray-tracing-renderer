#include "../par/include/image_par.hpp"
#include "../../common/include/constants.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

// Constructor tests
TEST(test_image_par, constructor_creates_correct_size) {
  ImagePar image(10, 20);
  EXPECT_EQ(image.width(), 10);
  EXPECT_EQ(image.height(), 20);
  EXPECT_EQ(image.total_pixels(), 200);
}

TEST(test_image_par, constructor_initializes_to_zero) {
  ImagePar image(2, 2);
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_EQ(image.get_red(i), 0);
    EXPECT_EQ(image.get_green(i), 0);
    EXPECT_EQ(image.get_blue(i), 0);
  }
}

// fill_from_double tests
TEST(test_image_par, fill_from_double_valid_data) {
  ImagePar image(1, 2);
  double const gamma = 2.0;
  std::vector<double> const r_data = {0.25, 0.5};
  std::vector<double> const g_data = {0.5, 0.75};
  std::vector<double> const b_data = {1.0, 0.0};

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

  auto expected_r0 = static_cast<uint8_t>(255.999 * std::pow(0.25, 1.0 / gamma));
  auto expected_g0 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_b0 = static_cast<uint8_t>(255.999 * std::pow(1.0, 1.0 / gamma));
  EXPECT_EQ(image.get_red(0), expected_r0);
  EXPECT_EQ(image.get_green(0), expected_g0);
  EXPECT_EQ(image.get_blue(0), expected_b0);
  EXPECT_EQ(image.get_blue(0), 255);

  auto expected_r1 = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  auto expected_g1 = static_cast<uint8_t>(255.999 * std::pow(0.75, 1.0 / gamma));
  auto expected_b1 = static_cast<uint8_t>(255.999 * std::pow(0.0, 1.0 / gamma));
  EXPECT_EQ(image.get_red(1), expected_r1);
  EXPECT_EQ(image.get_green(1), expected_g1);
  EXPECT_EQ(image.get_blue(1), expected_b1);
  EXPECT_EQ(image.get_blue(1), 0);
}

TEST(test_image_par, fill_from_double_default_gamma) {
  ImagePar image(1, 1);
  std::vector<double> const r = {0.5};
  std::vector<double> const g = {0.5};
  std::vector<double> const b = {0.5};
  
  RGBInputData input{&r, &g, &b};
  image.fill_from_double(input, Constants::Gamma, nullptr);

  double const default_gamma = 2.2;
  auto expected = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / default_gamma));
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_green(0), expected);
  EXPECT_EQ(image.get_blue(0), expected);
}

TEST(test_image_par, fill_from_double_extreme_values) {
  ImagePar image(3, 1);
  double const gamma = 2.0;

  std::vector<double> const r_data = {0.0, 1.0, 0.5};
  std::vector<double> const g_data = {1.0, 0.0, 0.5};
  std::vector<double> const b_data = {0.5, 0.5, 1.0};

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

  EXPECT_EQ(image.get_red(0), 0);
  EXPECT_EQ(image.get_green(0), 255);

  EXPECT_EQ(image.get_red(1), 255);
  EXPECT_EQ(image.get_green(1), 0);
}

TEST(test_image_par, fill_from_double_overwrites_previous_data) {
  ImagePar image(2, 1);
  std::vector<double> const r_data = {0.0, 0.0};
  std::vector<double> const g_data = {0.0, 0.0};
  std::vector<double> const b_data = {1.0, 1.0};

  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, Constants::Gamma, nullptr);

  EXPECT_EQ(image.get_red(0), 0);
  EXPECT_EQ(image.get_green(0), 0);
  EXPECT_NE(image.get_blue(0), 0);
}

TEST(test_image_par, fill_from_double_large_image) {
  size_t const width = 1920, height = 1080;
  size_t const size = width * height;
  ImagePar image(width, height);

  std::vector<double> const r_data(size, 0.5);
  std::vector<double> const g_data(size, 0.5);
  std::vector<double> const b_data(size, 0.5);

  double const gamma = 2.0;
  RGBInputData input{&r_data, &g_data, &b_data};
  image.fill_from_double(input, gamma, nullptr);

  auto expected = static_cast<uint8_t>(255.999 * std::pow(0.5, 1.0 / gamma));
  EXPECT_EQ(image.get_red(0), expected);
  EXPECT_EQ(image.get_red(size - 1), expected);
}

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
