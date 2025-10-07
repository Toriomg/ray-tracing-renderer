#include "../include/image_aos.hpp"
#include "../../common/include/ppm_writer.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include <cmath>
#include <stdexcept>

// Constructor para generar el array de píxeles del tamaño correcto proporcionado por el usuario
ImageAOS::ImageAOS(size_t width, size_t height)
    : pixels_(width * height, Pixel{}),  // Inicializamos todos los píxeles como negro
      width_(width), height_(height) {
  if (width == 0 or height == 0) {
    throw std::invalid_argument("Las dimensiones de la imagen no pueden ser cero");
  }
}

// Métodos para acceder al valor de un color específico dentro de un píxel
uint8_t ImageAOS::get_red(size_t row, size_t col) const {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  // Devolvemos el valor del componente rojo del píxel especificado usando la función de indice
  return pixels_[indice(row, col)].r;
}

uint8_t ImageAOS::get_green(size_t row, size_t col) const {
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  return pixels_[indice(row, col)].g;
}

uint8_t ImageAOS::get_blue(size_t row, size_t col) const {
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  return pixels_[indice(row, col)].b;
}

// Métodos para modificar un color específico dentro de un píxel
void ImageAOS::set_red(size_t row, size_t col, uint8_t value) {
  // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  pixels_[indice(row, col)].r = value;
}

void ImageAOS::set_green(size_t row, size_t col, uint8_t value) {
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  pixels_[indice(row, col)].g = value;
}

void ImageAOS::set_blue(size_t row, size_t col, uint8_t value) {
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  pixels_[indice(row, col)].b = value;
}

// Acceso directo al píxel completo (ventaja de AOS)
ImageAOS::Pixel const & ImageAOS::get_pixel(size_t row, size_t col) const {
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  return pixels_[indice(row, col)];
}

// Método para modificar los 3 componentes de color a la vez en un píxel concreto
void ImageAOS::set_pixel(size_t row, size_t col, uint8_t red, uint8_t green,
                         uint8_t blue) {  // NOLINT(readability-function-size)
  if (row >= height_ or col >= width_) {
    throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" +
                            std::to_string(row) +
                            ", col=" +
                            std::to_string(col));
  }
  Pixel & pixel = pixels_[indice(row, col)];
  pixel.r       = red;
  pixel.g       = green;
  pixel.b       = blue;
}

// Llenar toda la imagen con un color específico
void ImageAOS::fill_color(uint8_t red, uint8_t green, uint8_t blue) {
  for (auto & pixel : pixels_) {
    pixel = Pixel{red, green, blue};
  }
}

// Llenado desde datos float del renderizador
void ImageAOS::fill_from_float(std::vector<float> const & r_data, std::vector<float> const & g_data,
                               std::vector<float> const & b_data, float gamma) {
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
    pixels_[i].r = color_utils::float_to_uint8(color_utils::apply_gamma(r_data[i], gamma));
    pixels_[i].g = color_utils::float_to_uint8(color_utils::apply_gamma(g_data[i], gamma));
    pixels_[i].b = color_utils::float_to_uint8(color_utils::apply_gamma(b_data[i], gamma));
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

  return PPMWriter::write_ppm(filename, r_channel, g_channel, b_channel, width_, height_);
}
