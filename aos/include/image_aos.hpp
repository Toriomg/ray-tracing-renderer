#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <functional>

class ImageAOS {
public:
    // Estructura pública para representar un píxel
    struct Pixel {
        uint8_t r; 
        uint8_t g;  
        uint8_t b; 
        
        Pixel(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) 
            : r(red), g(green), b(blue) {}
    };

private:
    std::vector<Pixel> pixels_; 
    size_t width_; 
    size_t height_; 

    [[nodiscard]] size_t indice(size_t row, size_t col) const {
        return row * width_ + col;
    }

public:
    //  Creación de un array con el tamaño de pixeles en base a la altura y la anchura elegida
    ImageAOS(size_t width, size_t height);


    // Método para acceder al valor de un color de un pixel (Accedo al valor de rojo de un pixel concreto)
    [[nodiscard]] uint8_t get_red(size_t row, size_t col) const;
    [[nodiscard]] uint8_t get_green(size_t row, size_t col) const;
    [[nodiscard]] uint8_t get_blue(size_t row, size_t col) const;

    // Métodos para modificar valores de un color concreto a un píxel concreto
    void set_red(size_t row, size_t col, uint8_t value);
    void set_green(size_t row, size_t col, uint8_t value);
    void set_blue(size_t row, size_t col, uint8_t value);
    

    // Devuelve un pixel completo con los tres colores por su indice)
    [[nodiscard]] const Pixel& get_pixel(size_t row, size_t col) const;

    // Modificación de todos los colores de un píxel concreto de una sola vez
    void set_pixel(size_t row, size_t col, uint8_t red, uint8_t green, uint8_t blue); // NOLINT

    // Reestablecer todos los pixeles de la imagen a un color concreto 
    void fill_color(uint8_t red, uint8_t green, uint8_t blue);
    
    // Aplicar una operación concreta a todos los píxeles de la imagen
    void apply_to_all_pixels(std::function<void(Pixel&)> operation);

    // Rellena todos los pixeles como en la versión de SOA
    void fill_from_float(const std::vector<float>& r_data,
                        const std::vector<float>& g_data,
                        const std::vector<float>& b_data,
                        float gamma = 2.2F);

    // Rellena todos los pixeles a partir de un vector de arrays de 3 floats (rgb) para ajustarse mejor a AOS
    void fill_from_float_pixels(const std::vector<std::array<float, 3>>& float_pixels,
                            float gamma = 2.2F);

    // Getters para acceder a las dimensiones de la imagen
    [[nodiscard]] size_t width() const { return width_; }
    [[nodiscard]] size_t height() const { return height_; }
    [[nodiscard]] size_t total_pixels() const { return width_ * height_; }
    
    // Getters para acceder al array de píxeles y poder usarlo externamente sin poder modificarlo
    [[nodiscard]] const std::vector<Pixel>& get_pixels() const { return pixels_; }

    // Función para escribir la imagen en un archivo PPM
    [[nodiscard]] bool write_to_ppm(const std::string& filename) const;
};