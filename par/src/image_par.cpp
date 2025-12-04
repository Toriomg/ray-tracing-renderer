// no hacemos comprobaciones porque la mayoría ya se incluyen en el parser y en la configuración

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

// Constructor para generar los arrays de colores del tamaño correcto proporcionado por el usuario
ImagePar::ImagePar(size_t width, size_t height)
    : r_channel_(width * height, 0),  // Inicializamos todos con ceros
      g_channel_(width * height, 0), b_channel_(width * height, 0), width_(width), height_(height) {
}

// Métodos para comprobar los valores de un pixel concreto
uint8_t ImagePar::get_red(size_t index) const {
  return r_channel_[index];
}

uint8_t ImagePar::get_green(size_t index) const {
  return g_channel_[index];
}

uint8_t ImagePar::get_blue(size_t index) const {
  return b_channel_[index];
}

// Métodos para modificar el valor de un pixel dentro de los arrays de colores
void ImagePar::set_red(size_t index, double value, double gamma) {
  r_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImagePar::set_green(size_t index, double value, double gamma) {
  g_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImagePar::set_blue(size_t index, double value, double gamma) {
  b_channel_[index] = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

// Método para definir un color completo en un pixel concreto de una sola vez
void ImagePar::set_pixel(size_t index, Color const & color, double gamma) {
  set_red(index, color.x, gamma);
  set_green(index, color.y, gamma);
  set_blue(index, color.z, gamma);
}

void ImagePar::fill_from_double(RGBInputData const & input, double gamma,
                                ParallelSettings const * par_settings) {
  // Calculamos el tamaño esperado de los arrays a partir de las dimensiones de la imagen
  size_t const expected_size = width_ * height_;

  // Use default settings if none provided
  ParallelSettings const settings = par_settings != nullptr ? *par_settings : ParallelSettings{};

  // Create 1D range with custom grain size if specified
  tbb::blocked_range<size_t> range =
      (settings.grainSize > 0) ? tbb::blocked_range<size_t>(0, expected_size, settings.grainSize)
                               : tbb::blocked_range<size_t>(0, expected_size);

  // Lambda for pixel conversion
  auto convert_lambda = [&](tbb::blocked_range<size_t> const & r) {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      r_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.r)[i], gamma));
      g_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.g)[i], gamma));
      b_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.b)[i], gamma));
    }
  };

  // Select partitioner based on settings
  switch (settings.type) {
    case PartitionerType::Simple:
      tbb::parallel_for(range, convert_lambda, tbb::simple_partitioner());
      break;
    case PartitionerType::Static:
      tbb::parallel_for(range, convert_lambda, tbb::static_partitioner());
      break;
    case PartitionerType::Affinity:
    {
      static tbb::affinity_partitioner affinity_part;
      tbb::parallel_for(range, convert_lambda, affinity_part);
      break;
    }
    case PartitionerType::Auto:
    default:                    tbb::parallel_for(range, convert_lambda, tbb::auto_partitioner()); break;
  }
}

// Escritura a archivo PPM usando la clase PPMWriter
bool ImagePar::write_to_ppm(std::string const & filename) const {
  auto pixels = PPMWriter::Pixels(r_channel_, g_channel_, b_channel_);

  return PPMWriter::write_ppm(filename, pixels, width_, height_);
}
