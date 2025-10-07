#include "../include/image_soa.hpp"
#include "../../common/include/utilities/color_utils.hpp"
#include "../../common/include/ppm_writer.hpp"
#include <stdexcept>
#include <cmath>

// Constructor para generar los arrays de colores del tamaño correcto proporcionado por el usuario
ImageSOA::ImageSOA(size_t width, size_t height) 
    : r_channel_(width * height, 0),    // Inicializamos todos con ceros
    g_channel_(width * height, 0),
    b_channel_(width * height, 0),
    width_(width), height_(height)
{
    if (width == 0 or height == 0) {
        throw std::invalid_argument("Las dimensiones de la imagen no pueden ser cero");
    }
}

// Métodos para acceder al valor de un pixel dentro de los arrays de colores 
uint8_t ImageSOA::get_red(size_t row, size_t col) const {
    // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango 
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    // Devolvemos el valor del canal rojo para el píxel especificado usando la función de indice 
    return r_channel_[indice(row, col)];
}

uint8_t ImageSOA::get_green(size_t row, size_t col) const {
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    return g_channel_[indice(row, col)];
}

uint8_t ImageSOA::get_blue(size_t row, size_t col) const {
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    return b_channel_[indice(row, col)];
}

// Métodos para modificar el valor de un pixel dentro de los arrays de colores
void ImageSOA::set_red(size_t row, size_t col, uint8_t value) {
    // Aseguramos que las coordenadas están dentro de los límites establecidos y no fuera de rango
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    r_channel_[indice(row, col)] = value;
}

void ImageSOA::set_green(size_t row, size_t col, uint8_t value) {
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    g_channel_[indice(row, col)] = value;
}

void ImageSOA::set_blue(size_t row, size_t col, uint8_t value) {
    if (row >= height_ or col >= width_) {
        throw std::out_of_range("Coordenadas de píxel fuera de rango: fila=" + 
                               std::to_string(row) + ", col=" + std::to_string(col));
    }
    b_channel_[indice(row, col)] = value;
}

// Método para modificar los 3 arrays a la vez para definir un color completo en un pixel concreto de una sola vez
void ImageSOA::set_pixel(size_t row, size_t col, uint8_t red, uint8_t green, uint8_t blue) {// NOLINT(readability-function-size)
    set_red(row, col, red);
    set_green(row, col, green);
    set_blue(row, col, blue);
}

// Llenado desde datos float del renderizador
void ImageSOA::fill_from_float(const std::vector<float>& r_data,
                              const std::vector<float>& g_data,
                              const std::vector<float>& b_data,
                              float gamma) {
                                
    // Calculamos el tamaño esperado de los arrays a partir de las dimensiones de la imagen                            
    size_t expected_size = width_ * height_;
    
    // Verificamos que los datos proporcionados coinciden con las dimensiones de la imagen
    if (r_data.size() != expected_size or 
        g_data.size() != expected_size or 
        b_data.size() != expected_size) {
        throw std::invalid_argument("Los datos introducidos no coinciden con las dimensiones de la imagen");
    }

    // Aplicamos la corrección gamma a todos los valores y convertimos a uint8_t
    for (size_t i = 0; i < expected_size; ++i) {
        r_channel_[i] = color_utils::float_to_uint8(
            color_utils::apply_gamma(r_data[i], gamma));
        g_channel_[i] = color_utils::float_to_uint8(
            color_utils::apply_gamma(g_data[i], gamma));
        b_channel_[i] = color_utils::float_to_uint8(
            color_utils::apply_gamma(b_data[i], gamma));
    }
}

// Escritura a archivo PPM usando la clase PPMWriter
bool ImageSOA::write_to_ppm(const std::string& filename) const {
    return PPMWriter::write_ppm(filename, r_channel_, g_channel_, b_channel_, width_, height_);
}