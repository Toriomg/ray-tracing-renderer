#ifndef IMAGE_Par_HPP
#define IMAGE_Par_HPP

#include <../../common/include/constants.hpp>
#include <../../common/include/dataStructs/settings_structs.hpp>
#include <../../common/include/utilities/vec3.hpp>
#include <cstdint>
#include <string>
#include <vector>

// Structure to group RGB input data
struct RGBInputData {
  std::vector<double> const * r;
  std::vector<double> const * g;
  std::vector<double> const * b;
};

class ImagePar {
private:
  std::vector<uint8_t> r_channel_;
  std::vector<uint8_t> g_channel_;
  std::vector<uint8_t> b_channel_;
  size_t width_;
  size_t height_;

public:
  // Constructor
  ImagePar(size_t width, size_t height);

  [[nodiscard]] size_t indice(size_t row, size_t col) const { return row * width_ + col; }

  [[nodiscard]] uint8_t get_red(size_t index) const;
  [[nodiscard]] uint8_t get_green(size_t index) const;
  [[nodiscard]] uint8_t get_blue(size_t index) const;

  void set_red(size_t index, double value, double gamma = Constants::Gamma);
  void set_green(size_t index, double value, double gamma = Constants::Gamma);
  void set_blue(size_t index, double value, double gamma = Constants::Gamma);

  void set_pixel(size_t index, Color const & color, double gamma = Constants::Gamma);

  // Rama analysis/rendering: fill_from_double es SECUENCIAL (sin ParallelSettings)
  void fill_from_double(RGBInputData const & input, double gamma = Constants::Gamma);

  [[nodiscard]] size_t width() const { return width_; }
  [[nodiscard]] size_t height() const { return height_; }
  [[nodiscard]] size_t total_pixels() const { return width_ * height_; }

  // Rama analysis/rendering: write_to_ppm es SECUENCIAL (sin ParallelSettings)
  [[nodiscard]] bool write_to_ppm(std::string const & filename) const;
};

#endif
