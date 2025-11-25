#ifndef IMAGE_AOS_HPP
#define IMAGE_AOS_HPP

#include <../../common/include/constants.hpp>
#include <../../common/include/utilities/vec3.hpp>
#include <cstdint>
#include <string>
#include <vector>

class ImageAOS {
public:
  // Estructura pública para representar un píxel
  struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Pixel(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) { }
  };

private:
  std::vector<Pixel> pixels_;
  size_t width_;
  size_t height_;

public:
  //  Creación de un array con el tamaño de pixeles en base a la altura y la anchura elegida
  ImageAOS(size_t width, size_t height);

  // Método para calcular el índice lineal en el array a partir de las coordenadas 2D (fila,
  // columna)
  [[nodiscard]] size_t indice(size_t row, size_t col) const { return row * width_ + col; }

  // Devuelve un pixel completo con los tres colores por su indice)
  [[nodiscard]] Pixel const & get_pixel(size_t index) const;

  // Modificación de todos los colores de un píxel concreto de una sola vez
  void set_pixel(size_t index, Color const & color, double gamma = Constants::Gamma);

  // Rellena todos los pixeles como en la versión de AOS
  void fill_from_double(std::vector<double> const & r_data, std::vector<double> const & g_data,
                        std::vector<double> const & b_data, double gamma = Constants::Gamma);

  // Getters para acceder a las dimensiones de la imagen
  [[nodiscard]] size_t width() const { return width_; }

  [[nodiscard]] size_t height() const { return height_; }

  // Función para escribir la imagen en un archivo PPM
  [[nodiscard]] bool write_to_ppm(std::string const & filename) const;
};

#endif
