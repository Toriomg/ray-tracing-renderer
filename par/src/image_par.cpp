#include "../include/image_par.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <oneapi/tbb/partitioner.h>
#include <string>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <vector>

// TBB headers removidos - Rama analysis/writer usa procesado secuencial

// Constructor
ImagePar::ImagePar(size_t width, size_t height)
    : r_channel_(width * height, 0), g_channel_(width * height, 0), b_channel_(width * height, 0),
      width_(width), height_(height) { }

// Getters
uint8_t ImagePar::get_red(size_t index) const {
  return r_channel_[index];
}

uint8_t ImagePar::get_green(size_t index) const {
  return g_channel_[index];
}

uint8_t ImagePar::get_blue(size_t index) const {
  return b_channel_[index];
}

// Setters
void ImagePar::set_red(size_t index, double value, double gamma) {
  r_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImagePar::set_green(size_t index, double value, double gamma) {
  g_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImagePar::set_blue(size_t index, double value, double gamma) {
  b_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImagePar::set_pixel(size_t index, Color const & color, double gamma) {
  set_red(index, color.x, gamma);
  set_green(index, color.y, gamma);
  set_blue(index, color.z, gamma);
}

// NOLINTNEXTLINE
void ImagePar::fill_from_double(RGBInputData const & input, double gamma) {
  // Rama analysis/rendering: Post-procesado de imagen SECUENCIAL (control)
  // Solo el rendering es paralelo en esta rama

  size_t const total_pixels          = width_ * height_;
  std::vector<double> const & r_data = *(input.r);
  std::vector<double> const & g_data = *(input.g);
  std::vector<double> const & b_data = *(input.b);

  // BUCLE SECUENCIAL: Procesamos cada píxel uno por uno
  tbb::parallel_for(
      tbb::blocked_range<size_t>(0, total_pixels, 64),  // Grano de 64
      [&](tbb::blocked_range<size_t> const & range) {
        for (size_t i = range.begin(); i != range.end(); ++i) {
          r_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma(r_data[i], gamma));
          g_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma(g_data[i], gamma));
          b_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma(b_data[i], gamma));
        }
      },
      tbb::simple_partitioner()  // Usa simple_partitioner
  );
}

// Escritura a archivo PPM usando la clase PPMWriter (SECUENCIAL en esta rama)
bool ImagePar::write_to_ppm(std::string const & filename) const {
  return PPMWriter::write_ppm(filename, r_channel_, g_channel_, b_channel_, width_, height_);
}
