#include "../include/image_aos.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/constants.hpp>
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <stdexcept>
#include <vector>

// Constructor para generar el array de píxeles del tamaño correcto proporcionado por el usuario
ImageAOS::ImageAOS(size_t width, size_t height)
    : pixels_(width * height, Pixel{}),  // Inicializamos todos los píxeles como negro
      width_(width), height_(height) {
  if (width == 0 or height == 0) {
    throw std::invalid_argument("Las dimensiones de la imagen no pueden ser cero");
  }
}

// Métodos para acceder al valor de un color específico dentro de un píxel
uint8_t ImageAOS::get_red(size_t index) const {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  // Devolvemos el valor del componente rojo del píxel especificado usando la función de indice
  return pixels_[index].r;
}

uint8_t ImageAOS::get_green(size_t index) const {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  return pixels_[index].g;
}

uint8_t ImageAOS::get_blue(size_t index) const {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  return pixels_[index].b;
}

// Métodos para modificar un color específico dentro de un píxel
void ImageAOS::set_red(size_t index, double value, double gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  pixels_[index].r = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImageAOS::set_green(size_t index, double value, double gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  pixels_[index].g = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImageAOS::set_blue(size_t index, double value, double gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  pixels_[index].b = color_utils::double_to_uint8(color_utils::apply_gamma(value, gamma));
}

// Acceso directo al píxel completo (ventaja de AOS)
ImageAOS::Pixel const & ImageAOS::get_pixel(size_t index) const {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  return pixels_[index];
}

// Método para modificar los 3 componentes de color a la vez en un píxel concreto
void ImageAOS::set_pixel(size_t index, Color const & color, double gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  Pixel & pixel = pixels_[index];
  pixel.r       = color_utils::double_to_uint8(color_utils::apply_gamma(color.x, gamma));
  pixel.g       = color_utils::double_to_uint8(color_utils::apply_gamma(color.y, gamma));
  pixel.b       = color_utils::double_to_uint8(color_utils::apply_gamma(color.z, gamma));
}

// Llenar toda la imagen con un color específico
void ImageAOS::fill_color(Color const & color, double gamma) {
  for (auto & pixel : pixels_) {
    pixel = Pixel{color_utils::double_to_uint8(color_utils::apply_gamma(color.x, gamma)),
                  color_utils::double_to_uint8(color_utils::apply_gamma(color.y, gamma)),
                  color_utils::double_to_uint8(color_utils::apply_gamma(color.z, gamma))};
  }
}

// Llenado desde datos double del renderizador
void ImageAOS::fill_from_double(std::vector<double> const & r_data, std::vector<double> const & g_data,
                               std::vector<double> const & b_data, double gamma) {
  // Calculamos el tamaño esperado del array a partir de las dimensiones de la imagen
  size_t expected_size = width_ * height_;

  // Verificamos que los datos proporcionados coinciden con las dimensiones de la imagen
  if (r_data.size() != expected_size or
      g_data.size() != expected_size or
      b_data.size() != expected_size)
  {
    throw std::invalid_argument(
        "Los datos introducidos no coinciden con las dimensiones de la imagen");
  }

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
