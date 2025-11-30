#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <oneapi/tbb/partitioner.h>
#include <string>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
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
  {  // evitamos lanzar excepciones, solo imprimimos el error
    std::cerr << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
    return false;
  }
  std::ofstream file(filename);
  if (!file.is_open()) {  // abrimos archivo a escribir
    std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
    return false;
  }

  file << "P3\n";  // Cabecera para PPM P6 (binario)
  file << width << " " << height << "\n";
  file << "255\n";

  tbb::static_partitioner sp;
  std::vector<std::string> output_lines(total_pixels);
  tbb::parallel_for(
      tbb::blocked_range<size_t>(0, total_pixels),
      [&](tbb::blocked_range<size_t> const & range) {
        for (size_t i = range.begin(); i != range.end(); ++i) {
          output_lines[i] = std::to_string(static_cast<int>(pixels.r_channel[i])) +
                            " " +
                            std::to_string(static_cast<int>(pixels.g_channel[i])) +
                            " " +
                            std::to_string(static_cast<int>(pixels.b_channel[i])) +
                            "\n";
        }
      },
      sp);                                  // procesamos en paralelo
  for (auto const & line : output_lines) {  // escribimos en secuencial
    file << line;
  }
  file.close();
  return true;
}
