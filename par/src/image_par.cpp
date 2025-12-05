#include "../include/image_par.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Rama analysis/image: fill_from_double usa TBB PARALLEL
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>

// Constructor
ImagePar::ImagePar(size_t width, size_t height)
    : r_channel_(width * height, 0), g_channel_(width * height, 0),
      b_channel_(width * height, 0), width_(width), height_(height) { }

// Getters
uint8_t ImagePar::get_red(size_t index) const { return r_channel_[index]; }

uint8_t ImagePar::get_green(size_t index) const { return g_channel_[index]; }

uint8_t ImagePar::get_blue(size_t index) const { return b_channel_[index]; }

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

// Rama analysis/image: fill_from_double es PARALELO (TBB)
void ImagePar::fill_from_double(RGBInputData const & input, double gamma,
                                ParallelSettings const * par_settings) {
  size_t const total_pixels = width_ * height_;

  ParallelSettings settings = (par_settings != nullptr) ? *par_settings : ParallelSettings{};

  // Functor para procesamiento paralelo
  auto process_pixels = [&](tbb::blocked_range<size_t> const & range) {
    for (size_t i = range.begin(); i != range.end(); ++i) {
      r_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.r)[i], gamma));
      g_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.g)[i], gamma));
      b_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.b)[i], gamma));
    }
  };

  // Configurar rango con grain size
  tbb::blocked_range<size_t> range = (settings.grainSize > 0)
                                         ? tbb::blocked_range<size_t>(0, total_pixels, settings.grainSize)
                                         : tbb::blocked_range<size_t>(0, total_pixels);

  // Seleccionar particionador según configuración
  switch (settings.type) {
    case PartitionerType::Simple:
      tbb::parallel_for(range, process_pixels, tbb::simple_partitioner());
      break;
    case PartitionerType::Static:
      tbb::parallel_for(range, process_pixels, tbb::static_partitioner());
      break;
    case PartitionerType::Affinity: {
      static tbb::affinity_partitioner affinity_part;
      tbb::parallel_for(range, process_pixels, affinity_part);
      break;
    }
    default:  // Auto
      tbb::parallel_for(range, process_pixels, tbb::auto_partitioner());
      break;
  }
}

// Escritura a archivo PPM (SECUENCIAL en esta rama)
bool ImagePar::write_to_ppm(std::string const & filename) const {
  return PPMWriter::write_ppm(filename, r_channel_, g_channel_, b_channel_, width_, height_);
}
