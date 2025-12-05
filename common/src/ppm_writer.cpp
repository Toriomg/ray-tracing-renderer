#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Versión SIMPLE para rama analysis/rendering
// En esta rama SOLO el rendering es paralelo, el writer es secuencial

bool PPMWriter::write_ppm(std::string const & filename,
                          std::vector<uint8_t> const & r_channel,
                          std::vector<uint8_t> const & g_channel,
                          std::vector<uint8_t> const & b_channel,
                          size_t width, size_t height) {
  size_t const total_pixels = width * height;

  // Validar dimensiones
  if (r_channel.size() != total_pixels or g_channel.size() != total_pixels or
      b_channel.size() != total_pixels) {
    std::cerr << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
    return false;
  }

  // Abrir archivo
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
    return false;
  }

  // Escribir cabecera PPM P3
  file << "P3\n";
  file << width << " " << height << "\n";
  file << "255\n";

  // Escribir píxeles SECUENCIALMENTE (no paralelizado en esta rama)
  for (size_t i = 0; i < total_pixels; ++i) {
    file << static_cast<int>(r_channel[i]) << " "
         << static_cast<int>(g_channel[i]) << " "
         << static_cast<int>(b_channel[i]) << "\n";
  }

  return file.good();
}
