#ifndef IMAGE_Par_HPP
#define IMAGE_Par_HPP

#include <../../common/include/constants.hpp>
#include <../../common/include/dataStructs/settings_structs.hpp>
#include <../../common/include/utilities/vec3.hpp>
#include <cstdint>
#include <string>
#include <vector>

// Structure to group RGB input data
struct RGBInputData {
  std::vector<double> const * r;
  std::vector<double> const * g;
  std::vector<double> const * b;
};

class ImagePar {
private:
  std::vector<uint8_t>
      r_channel_;  // Array que almacena el valor de rojo para todos los píxeles de la imagen
  std::vector<uint8_t>
      g_channel_;  // Array que almacena el valor de verde para todos los píxeles de la imagen
  std::vector<uint8_t>
      b_channel_;  // Array que almacena el valor de azul para todos los píxeles de la imagen
  size_t width_;   // Ancho de la imagen
  size_t height_;  // Alto de la imagen

public:
  // Constructor: crea los 3 arrays del tamaño adecuado
  ImagePar(size_t width, size_t height);

  // Función para calcular el índice de un pixel en los arrays de colores
  [[nodiscard]] size_t indice(size_t row, size_t col) const { return row * width_ + col; }

  // Métodos para comprobar los valores de un pixel concreto
  [[nodiscard]] uint8_t get_red(size_t index) const;
  [[nodiscard]] uint8_t get_green(size_t index) const;
  [[nodiscard]] uint8_t get_blue(size_t index) const;

  // Métodos para modificar valores de color a un pixel concreto en cada array
  void set_red(size_t index, double value, double gamma = Constants::Gamma);
  void set_green(size_t index, double value, double gamma = Constants::Gamma);
  void set_blue(size_t index, double value, double gamma = Constants::Gamma);

  // Función que permite modificar todos los colores a la vez de un solo pixel (modificar los 3
  // arrays para definir un color)
  void set_pixel(size_t index, Color const & color, double gamma = Constants::Gamma);

  // Permite llenar todos los arrays a partir de datos en double con valores de 0 a 1 a valores
  // válidos del 0 al 255
  void fill_from_double(RGBInputData const & input, double gamma = Constants::Gamma,
                        ParallelSettings const * par_settings = nullptr);

  // Métodos para recibir las dimensiones de la imagen
  [[nodiscard]] size_t width() const { return width_; }

  [[nodiscard]] size_t height() const { return height_; }

  [[nodiscard]] size_t total_pixels() const { return width_ * height_; }

  // Función para escribir la imagen en un archivo PPM
  [[nodiscard]] bool write_to_ppm(std::string const & filename) const;
};

#endif
