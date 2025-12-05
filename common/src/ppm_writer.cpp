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
                          size_t height, ParallelSettings const * settings) {
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

  // Use default settings if none provided
  ParallelSettings const default_settings{};
  ParallelSettings const & active_settings = settings != nullptr ? *settings : default_settings;

  // Create blocked_range with custom grain size if specified
  tbb::blocked_range<size_t> range =
      (active_settings.grainSize > 0)
          ? tbb::blocked_range<size_t>(0, total_pixels, active_settings.grainSize)
          : tbb::blocked_range<size_t>(0, total_pixels);

  std::vector<std::string> output_lines(total_pixels);

  // Lambda for parallel conversion
  auto convert_lambda = [&](tbb::blocked_range<size_t> const & r) {
    for (size_t i = r.begin(); i != r.end(); ++i) {
      output_lines[i] = std::to_string(static_cast<int>(pixels.r_channel[i])) +
                        " " +
                        std::to_string(static_cast<int>(pixels.g_channel[i])) +
                        " " +
                        std::to_string(static_cast<int>(pixels.b_channel[i])) +
                        "\n";
    }
  };

  // Select partitioner based on settings
  switch (active_settings.type) {
    case PartitionerType::Simple:
      tbb::parallel_for(range, convert_lambda, tbb::simple_partitioner());
      break;
    case PartitionerType::Static:
      tbb::parallel_for(range, convert_lambda, tbb::static_partitioner());
      break;
    case PartitionerType::Affinity:
    {
      static tbb::affinity_partitioner affinity_part;
      tbb::parallel_for(range, convert_lambda, affinity_part);
      break;
    }
    case PartitionerType::Auto:
    default:                    tbb::parallel_for(range, convert_lambda, tbb::auto_partitioner()); break;
  }

  // Escribimos en secuencial
  for (auto const & line : output_lines) {
    file << line;
  }
  file.close();
  return true;
}
