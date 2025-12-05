#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Rama analysis/image: Writer es SECUENCIAL (sin TBB)

// NOLINTNEXTLINE(readability-function-size)
bool PPMWriter::write_ppm(std::string const & filename, std::vector<uint8_t> const & r_channel,
                          std::vector<uint8_t> const & g_channel,
                          std::vector<uint8_t> const & b_channel, size_t width, size_t height) {
  size_t const total_pixels = width * height;

  // Validar dimensiones
  if (r_channel.size() != total_pixels or  // NOLINT(readability-operators-representation)
      g_channel.size() != total_pixels or  // NOLINT(readability-operators-representation)
      b_channel.size() != total_pixels)
  {
    std::cerr << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
    return false;
  }

  // Abrir archivo
  std::ofstream file(filename);
  if (not file.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
    return false;
  }

  // Escribir cabecera PPM P3
  file << "P3\n" << width << " " << height << "\n255\n";

  // Escribir píxeles SECUENCIALMENTE
  for (size_t i = 0; i < total_pixels; ++i) {
    file << static_cast<int>(r_channel[i]) << " " << static_cast<int>(g_channel[i]) << " "
         << static_cast<int>(b_channel[i]) << "\n";
  }

  return file.good();
}
