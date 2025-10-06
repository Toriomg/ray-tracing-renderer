#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class PPMWriter {
public:
    // Escribe una imágen en formato PPM P3 recibiendo los 3 arrays de colores y las dimensiones de la imagen
    static bool write_ppm(const std::string& filename,
                         const std::vector<uint8_t>& r_channel,
                         const std::vector<uint8_t>& g_channel,
                         const std::vector<uint8_t>& b_channel,
                         size_t width, size_t height) {
        
        // Checkeamos que el número de pixeles coincide con el tamaño de los arrays que se han definido
        size_t total_pixels = width * height;
        if (r_channel.size() != total_pixels || 
            g_channel.size() != total_pixels || 
            b_channel.size() != total_pixels) {
            throw std::invalid_argument("El tamaño de los arrays no se corresponde con las dimensiones de la imagen");
        }
        
        // Abre un archivo para escribir
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("No se puede abrir el archivo para escritura: " + filename);
        }
        
        try {
            // Escribir cabecera PPM
            file << "P3\n"; // primera línea: cadena P3 + salto de linea      
            file << width << " " << height << "\n"; // segunda línea: número de lineas y columnas + salto de linea
            file << "255\n";    // tercera línea: valor máximo de color + salto de linea
            
            // Resto de lineas definen los valores de los colores de cada pixel
            for (size_t i = 0; i < total_pixels; ++i) {
                file << static_cast<int>(r_channel[i]) << " "
                     << static_cast<int>(g_channel[i]) << " " 
                     << static_cast<int>(b_channel[i]) << "\n";
            }
            
            file.close();
            return true;
          //cerramos el archivo y mandamos flag para definir si la operación se ha realizado correctamente  
        } catch (const std::exception& e) {
            if (file.is_open()) {
                file.close();
            }
            throw std::runtime_error("Error de escritura en el archivo PPM: " + std::string(e.what()));
        }
    }
};