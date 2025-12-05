#include "ppm_writer.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <oneapi/tbb/partitioner.h>
#include <string>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <vector>

namespace {

  using Pixels = PPMWriter::Pixels;

  bool validate_pixel_dimensions(Pixels const & pixels) {
    size_t const total_pixels = pixels.width * pixels.height;
    if (pixels.r_channel.size() != total_pixels or
        pixels.g_channel.size() != total_pixels or
        pixels.b_channel.size() != total_pixels)
    {
      std::cerr
          << "Error: El tamaño de los canales no coincide con las dimensiones de la imagen.\n";
      return false;
    }
    return true;
  }

  bool write_ppm_header(std::ofstream & file, size_t width, size_t height) {
    file << "P3\n";
    file << width << " " << height << "\n";
    file << "255\n";
    return file.good();
  }

  void convert_pixels_parallel(Pixels const & pixels, size_t total_pixels,
                               std::vector<std::string> & output_lines,
                               ParallelSettings const & active_settings) {
    tbb::blocked_range<size_t> range =
        (active_settings.grainSize > 0)
            ? tbb::blocked_range<size_t>(0, total_pixels, active_settings.grainSize)
            : tbb::blocked_range<size_t>(0, total_pixels);

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
  }

  void write_output_lines(std::ofstream & file, std::vector<std::string> const & output_lines) {
    for (auto const & line : output_lines) {
      file << line;
    }
  }

  bool process_and_write_pixels(std::ofstream & file, Pixels const & pixels, size_t total_pixels,
                                ParallelSettings const & active_settings) {
    std::vector<std::string> output_lines(total_pixels);
    convert_pixels_parallel(pixels, total_pixels, output_lines, active_settings);
    write_output_lines(file, output_lines);
    return true;
  }

  bool open_and_write_header(std::string const & filename, size_t width, size_t height,
                             std::ofstream & file) {
    file.open(filename);
    if (!file.is_open()) {
      std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filename << "\n";
      return false;
    }
    return write_ppm_header(file, width, height);
  }

  ParallelSettings const & get_active_settings(ParallelSettings const * settings) {
    static ParallelSettings const default_settings{};
    return settings != nullptr ? *settings : default_settings;
  }

}  // namespace

// Escribe una imágen en formato PPM P3 recibiendo los 3 arrays de colores y las dimensiones de la
// imagen
bool PPMWriter::write_ppm(std::string const & filename, Pixels const & pixels,
                          ParallelSettings const * settings) {
  if (!validate_pixel_dimensions(pixels)) {
    return false;
  }

  std::ofstream file;
  if (!open_and_write_header(filename, pixels.width, pixels.height, file)) {
    return false;
  }

  return process_and_write_pixels(file, pixels, pixels.width * pixels.height,
                                  get_active_settings(settings));
}
