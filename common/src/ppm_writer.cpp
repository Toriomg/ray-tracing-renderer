#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Escribe una imágen en formato PPM P3 recibiendo los 3 arrays de colores y las dimensiones de la
// imagen
bool PPMWriter::write_ppm(std::string const & filename, Pixels const & pixels, size_t width,
                          size_t height) {
  // Checkeamos que el número de pixeles coincide con el tamaño de los arrays que se han definido
  size_t const total_pixels = width * height;
  if (pixels.r_channel.size() != total_pixels or
      pixels.g_channel.size() != total_pixels or
      pixels.b_channel.size() != total_pixels)
  {
    // Es mejor no lanzar excepciones aquí si la función devuelve bool.
    // Imprime un error y devuelve false.
    std::cerr << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
    return false;
  }

  // Abre un archivo para escribir
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
    return false;
  }

  // Cabecera para PPM P6 (binario)
  file << "P3\n";
  file << width << " " << height << "\n";
  file << "255\n";

  for (size_t i = 0; i < total_pixels; ++i) {
    // Escribe los valores numéricos como texto, separados por espacios
    file << static_cast<int>(pixels.r_channel[i]) << " " << static_cast<int>(pixels.g_channel[i])
         << " " << static_cast<int>(pixels.b_channel[i]) << "\n";
  }

  file.close();
  return true;
}
