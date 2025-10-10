#include "../include/image_soa.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <../../common/include/constants.hpp>
#include <../../common/include/utilities/vec3.hpp>
#include <cmath>
#include <stdexcept>
#include <vector>

// Constructor para generar los arrays de colores del tamaño correcto proporcionado por el usuario
ImageSOA::ImageSOA(size_t width, size_t height)
    : r_channel_(width * height, 0),  // Inicializamos todos con ceros
      g_channel_(width * height, 0), b_channel_(width * height, 0), width_(width), height_(height) {
  if (width == 0 or height == 0) {
    throw std::invalid_argument("Las dimensiones de la imagen no pueden ser cero");
  }
}

[[nodiscard]] size_t ImageSOA::indice(size_t row, size_t col) const {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (row >= height_ or col >= width_) {
    throw std::out_of_range(
        "Coordenadas fuera de rango: (" + std::to_string(row) + ", " + std::to_string(col) + ")");
  }
  return row * width_ + col;
}

// Métodos para acceder al valor de un pixel dentro de los arrays de colores
uint8_t ImageSOA::get_red(size_t index) const {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  // Devolvemos el valor del canal rojo para el píxel especificado usando la función de indice
  return r_channel_[index];
}

uint8_t ImageSOA::get_green(size_t index) const {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  return g_channel_[index];
}

uint8_t ImageSOA::get_blue(size_t index) const {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  return b_channel_[index];
}

// Métodos para modificar el valor de un pixel dentro de los arrays de colores
void ImageSOA::set_red(size_t index, float value, float gamma) {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  r_channel_[index] = color_utils::float_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImageSOA::set_green(size_t index, float value, float gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  if (value < 0.0F or value > 255.0F) {
    throw std::out_of_range("Valor de color fuera de rango: " + std::to_string(value));
  }
  g_channel_[index] = color_utils::float_to_uint8(color_utils::apply_gamma(value, gamma));
}

void ImageSOA::set_blue(size_t index, float value, float gamma) {
  if (index >= total_pixels()) {
    throw std::out_of_range("Índice de píxel fuera de rango: " + std::to_string(index));
  }
  if (value < 0.0F or value > 255.0F) {
    throw std::out_of_range("Valor de color fuera de rango: " + std::to_string(value));
  }
  b_channel_[index] = color_utils::float_to_uint8(color_utils::apply_gamma(value, gamma));
}

// Método para modificar los 3 arrays a la vez para definir un color completo en un pixel concreto
// de una sola vez
void ImageSOA::set_pixel(size_t index, Color const & color, float gamma) {
  set_red(index, color.x, gamma);
  set_green(index, color.y, gamma);
  set_blue(index, color.z, gamma);
}

// Llenado desde datos float del renderizador
void ImageSOA::fill_from_float(std::vector<float> const & r_data, std::vector<float> const & g_data,
                               std::vector<float> const & b_data, float gamma) {
  // Calculamos el tamaño esperado de los arrays a partir de las dimensiones de la imagen
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
    r_channel_[i] = color_utils::float_to_uint8(color_utils::apply_gamma(r_data[i], gamma));
    g_channel_[i] = color_utils::float_to_uint8(color_utils::apply_gamma(g_data[i], gamma));
    b_channel_[i] = color_utils::float_to_uint8(color_utils::apply_gamma(b_data[i], gamma));
  }
}

// Escritura a archivo PPM usando la clase PPMWriter
bool ImageSOA::write_to_ppm(std::string const & filename) const {
  return PPMWriter::write_ppm(filename, r_channel_, g_channel_, b_channel_, width_, height_);
}
