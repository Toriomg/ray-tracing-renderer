#ifndef RAW_IMAGE_HPP
#define RAW_IMAGE_HPP

#include "utilities/vec3.hpp"
#include <cstddef>
#include <vector>

// RawImage: Buffer intermedio que almacena colores en formato double (0.0 - 1.0)
// Rama analysis/image: Este buffer se llena SECUENCIALMENTE por el rendering,
// luego ImagePar lo procesa PARALELAMENTE aplicando gamma y conversión a uint8_t
class RawImage {
private:
  std::vector<double> r_channel_;  // Red channel (0.0 - 1.0)
  std::vector<double> g_channel_;  // Green channel (0.0 - 1.0)
  std::vector<double> b_channel_;  // Blue channel (0.0 - 1.0)
  size_t width_;
  size_t height_;

public:
  // Constructor
  RawImage(size_t width, size_t height)
      : r_channel_(width * height, 0.0), g_channel_(width * height, 0.0),
        b_channel_(width * height, 0.0), width_(width), height_(height) { }

  // Calculate index for pixel at (row, col)
  [[nodiscard]] size_t indice(size_t row, size_t col) const { return row * width_ + col; }

  // Set pixel color (stores raw double values)
  void set_pixel(size_t index, Color const & color) {
    r_channel_[index] = color.x;
    g_channel_[index] = color.y;
    b_channel_[index] = color.z;
  }

  // Getters for dimensions
  [[nodiscard]] size_t width() const { return width_; }

  [[nodiscard]] size_t height() const { return height_; }

  [[nodiscard]] size_t total_pixels() const { return width_ * height_; }

  // Access to raw channels (for ImagePar processing)
  [[nodiscard]] std::vector<double> const & r_channel() const { return r_channel_; }

  [[nodiscard]] std::vector<double> const & g_channel() const { return g_channel_; }

  [[nodiscard]] std::vector<double> const & b_channel() const { return b_channel_; }
};

#endif
