#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <vector>

// Escribe una imágen en formato PPM P3 recibiendo los 3 arrays de colores y las dimensiones de la
// imagen
bool PPMWriter::write_ppm(std::string const & filename, Pixels const & pixels, size_t width,
                          size_t height) {
  size_t const total_pixels = width * height;
  if (pixels.r_channel.size() != total_pixels or
      pixels.g_channel.size() != total_pixels or
      pixels.b_channel.size() != total_pixels)
  {
    std::cerr << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
    return false;
  }
  std::ofstream file(filename);  // Abre un archivo para escribir
  if (!file.is_open()) {
    std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
    return false;
  }
  file << "P3\n";  // Cabecera para PPM P3 (texto)
  file << width << " " << height << "\n";
  file << "255\n";

  size_t const num_threads = std::thread::hardware_concurrency();  // control numero hilos
  size_t const chunk_size  = std::max<size_t>(1, total_pixels / (num_threads * 4));  // granularidad

  std::vector<std::string> buffers;
  buffers.resize((total_pixels + chunk_size - 1) / chunk_size);

  tbb::parallel_for(tbb::blocked_range<size_t>(0, total_pixels, chunk_size),
                    [&](tbb::blocked_range<size_t> const & range) {
                      std::stringstream ss;
                      for (size_t i = range.begin(); i != range.end(); ++i) {  // escritura
                        ss << static_cast<int>(pixels.r_channel[i]) << " "
                           << static_cast<int>(pixels.g_channel[i]) << " "
                           << static_cast<int>(pixels.b_channel[i]) << "\n";
                      }
                      buffers[range.begin() / chunk_size] = ss.str();
                    });

  for (auto const & buffer : buffers) {  // Write all buffers sequentially to the file
    file << buffer;
  }
  file.close();
  return true;
}
