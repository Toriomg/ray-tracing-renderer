#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include "ppm_writer.hpp"

// Escribe una imágen en formato PPM P3 recibiendo los 3 arrays de colores y las dimensiones de la imagen
bool PPMWriter::write_ppm(const std::string& filename,// NOLINT(readability-function-size)
                     const std::vector<uint8_t>& r_channel,
                     const std::vector<uint8_t>& g_channel,
                     const std::vector<uint8_t>& b_channel,
                     size_t width, size_t height)
{
    // Checkeamos que el número de pixeles coincide con el tamaño de los arrays que se han definido
    size_t total_pixels = width * height;
    if (r_channel.size() != total_pixels or
    g_channel.size() != total_pixels or
    b_channel.size() != total_pixels) {
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
    file << "P6\n";
    file << width << " " << height << "\n";
    file << "255\n";

    // Escribir datos binarios
    for (size_t i = 0; i < total_pixels; ++i) {
        file.put(static_cast<char>(r_channel[i]));
        file.put(static_cast<char>(g_channel[i]));
        file.put(static_cast<char>(b_channel[i]));
    }

    file.close();
    return true;
}