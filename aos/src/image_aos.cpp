// no hacemos comprobaciones porque la mayoría ya se incluyen en el parser y en la configuración

#include "../include/image_aos.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Constructor para generar el array de píxeles del tamaño correcto proporcionado por el usuario
ImageAOS::ImageAOS(size_t width, size_t height)
    : pixels_(width * height, Pixel{}),  // Inicializamos todos los píxeles como negro
      width_(width), height_(height) { }

// Acceso directo al píxel completo (ventaja de AOS)
ImageAOS::Pixel const & ImageAOS::get_pixel(size_t index) const {
  return pixels_[index];
}

// Método para modificar los 3 componentes de color a la vez en un píxel concreto
void ImageAOS::set_pixel(size_t index, Color const & color, double gamma) {
  Pixel & pixel = pixels_[index];
  pixel.r       = color_utils::double_to_uint8(color_utils::apply_gamma(color.x, gamma));
  pixel.g       = color_utils::double_to_uint8(color_utils::apply_gamma(color.y, gamma));
  pixel.b       = color_utils::double_to_uint8(color_utils::apply_gamma(color.z, gamma));
}

// Llenado desde datos double del renderizador
void ImageAOS::fill_from_double(std::vector<double> const & r_data,
                                std::vector<double> const & g_data,
                                std::vector<double> const & b_data, double gamma) {
  // Calculamos el tamaño esperado del array a partir de las dimensiones de la imagen
  size_t const expected_size = width_ * height_;

  // Aplicamos la corrección gamma a todos los valores y convertimos a uint8_t
  for (size_t i = 0; i < expected_size; ++i) {
    pixels_[i].r = color_utils::double_to_uint8(color_utils::apply_gamma(r_data[i], gamma));
    pixels_[i].g = color_utils::double_to_uint8(color_utils::apply_gamma(g_data[i], gamma));
    pixels_[i].b = color_utils::double_to_uint8(color_utils::apply_gamma(b_data[i], gamma));
  }
}

// Escritura a archivo PPM usando la clase PPMWriter
bool ImageAOS::write_to_ppm(std::string const & filename) const {
  // Para AOS, necesitamos extraer los canales individuales para PPMWriter
  std::vector<uint8_t> r_channel, g_channel, b_channel;
  r_channel.reserve(pixels_.size());
  g_channel.reserve(pixels_.size());
  b_channel.reserve(pixels_.size());

  for (auto const & pixel : pixels_) {
    r_channel.push_back(pixel.r);
    g_channel.push_back(pixel.g);
    b_channel.push_back(pixel.b);
  }
  auto pixels = PPMWriter::Pixels(r_channel, g_channel, b_channel);

  return PPMWriter::write_ppm(filename, pixels, width_, height_);
}
