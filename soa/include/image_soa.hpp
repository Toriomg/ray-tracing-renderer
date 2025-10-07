#pragma once

#include <cstdint>
#include <vector>
#include <string>

class ImageSOA {
private:
    std::vector<uint8_t> r_channel_; // Array que almacena el valor de rojo para todos los píxeles de la imagen 
    std::vector<uint8_t> g_channel_; // Array que almacena el valor de verde para todos los píxeles de la imagen
    std::vector<uint8_t> b_channel_; // Array que almacena el valor de azul para todos los píxeles de la imagen
    size_t width_;  // Ancho de la imagen
    size_t height_; // Alto de la imagen

    // Función para calcular el índice de un pixel en los arrays de colores
    [[nodiscard]]size_t indice(size_t row, size_t col) const {
        return row * width_ + col;
    }

public:
    // Constructor: crea los 3 arrays del tamaño adecuado
    ImageSOA(size_t width, size_t height);
    
    // Métodos para acceder a los valores de cada color (cada array)
    [[nodiscard]]uint8_t get_red(size_t row, size_t col) const;
    [[nodiscard]]uint8_t get_green(size_t row, size_t col) const;
    [[nodiscard]]uint8_t get_blue(size_t row, size_t col) const;
    
    // Métodos para modificar valores de color a un pixel concreto en cada array 
    void set_red(size_t row, size_t col, uint8_t value);
    void set_green(size_t row, size_t col, uint8_t value);
    void set_blue(size_t row, size_t col, uint8_t value);
    
    // MFunción que permite modificar todos los colores a la vez de un solo pixel (modificar los 3 arrays para definir un color)
    void set_pixel(size_t row, size_t col, uint8_t red, uint8_t green, uint8_t blue);

    // Permite llenar los arrays a partir de datos en float con valores de 0 a 1 a valores válidos del 0 al 255
    void fill_from_float(const std::vector<float>& r_data,
                        const std::vector<float>& g_data,
                        const std::vector<float>& b_data,
                        float gamma = 2.2F);
    
    // Métodos para recibir las dimensiones de la imagen 
    [[nodiscard]]size_t width() const { return width_; }
    [[nodiscard]]size_t height() const { return height_; }
    [[nodiscard]]size_t total_pixels() const { return width_ * height_; }
    
    // Getters para acceder a todos los arrays de colores y poder usarlos externamente sin poder modificarlos 
    [[nodiscard]]const std::vector<uint8_t>& get_r_channel() const { return r_channel_; }
    [[nodiscard]]const std::vector<uint8_t>& get_g_channel() const { return g_channel_; }
    [[nodiscard]]const std::vector<uint8_t>& get_b_channel() const { return b_channel_; }

    // Función para escribir la imagen en un archivo PPM
    [[nodiscard]]bool write_to_ppm(const std::string& filename) const;
};
