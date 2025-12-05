// no hacemos comprobaciones porque la mayoría ya se incluyen en el parser y en la configuración

#include "../include/image_par.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// TBB headers removidos - Rama analysis/writer usa procesado secuencial

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
  // Rama analysis/writer: Post-procesado SECUENCIAL
  // Ignoramos par_settings para mantener compatibilidad de firma
  (void) par_settings;

  // Calculamos el tamaño total de píxeles
  size_t const total_pixels = width_ * height_;

  // BUCLE SECUENCIAL: Procesamos cada píxel uno por uno
  for (size_t i = 0; i < total_pixels; ++i) {
    r_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.r)[i], gamma));
    g_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.g)[i], gamma));
    b_channel_[i] = color_utils::double_to_uint8(color_utils::apply_gamma((*input.b)[i], gamma));
  }
}

// Escritura a archivo PPM usando la clase PPMWriter
bool ImagePar::write_to_ppm(std::string const & filename, ParallelSettings const * settings) const {
  auto pixels = PPMWriter::Pixels(r_channel_, g_channel_, b_channel_, width_, height_);

  return PPMWriter::write_ppm(filename, pixels, settings);
}
