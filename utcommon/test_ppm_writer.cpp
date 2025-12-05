#include "ppm_writer.hpp"
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA PPMWriter
// ============================================================================

class PPMWriterTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza común: eliminar cualquier archivo temporal que pueda haber quedado
    static_cast<void>(std::remove("test_output.ppm"));
    static_cast<void>(std::remove("test_error.ppm"));
  }
};

// ============================================================================
// TESTS PARA PPMWriter::write_ppm
// ============================================================================

// Test 1: Escritura válida de imagen
TEST_F(PPMWriterTest, ValidImageWrite) {
  // Configuración
  std::string const filename = "test_output.ppm";
  size_t const width         = 2;
  size_t const height        = 1;

  // Crear estructura Pixels usando PPMWriter::Pixels
  // Los canales son std::vector<uint8_t>
  std::vector<uint8_t> const r_channel = {255, 0};
  std::vector<uint8_t> const g_channel = {0, 128};
  std::vector<uint8_t> const b_channel = {0, 255};

  PPMWriter::Pixels const pixels(r_channel, g_channel, b_channel, width, height);

  // Llamar a write_ppm
  bool const result = PPMWriter::write_ppm(filename, pixels);

  // Verificar que la escritura fue exitosa
  ASSERT_TRUE(result) << "write_ppm debería retornar true para datos válidos";

  // Verificar el contenido del archivo
  std::ifstream file(filename);
  ASSERT_TRUE(file.is_open()) << "El archivo debería haberse creado correctamente";

  std::string line;

  // Verificar cabecera PPM P3
  std::getline(file, line);
  ASSERT_EQ(line, "P3") << "Primera línea debería ser 'P3' (formato PPM texto)";

  // Verificar dimensiones
  std::getline(file, line);
  ASSERT_EQ(line, "2 1") << "Segunda línea debería ser '2 1' (ancho alto)";

  // Verificar valor máximo de color
  std::getline(file, line);
  ASSERT_EQ(line, "255") << "Tercera línea debería ser '255' (max color value)";

  // Verificar datos de píxeles
  int r = 0;
  int g = 0;
  int b = 0;

  // Primer píxel: (255, 0, 0)
  file >> r >> g >> b;
  ASSERT_EQ(r, 255) << "Primer píxel: componente R debería ser 255";
  ASSERT_EQ(g, 0) << "Primer píxel: componente G debería ser 0";
  ASSERT_EQ(b, 0) << "Primer píxel: componente B deber ser 0";

  // Segundo píxel: (0, 128, 255)
  file >> r >> g >> b;
  ASSERT_EQ(r, 0) << "Segundo píxel: componente R debería ser 0";
  ASSERT_EQ(g, 128) << "Segundo píxel: componente G debería ser 128";
  ASSERT_EQ(b, 255) << "Segundo píxel: componente B debería ser 255";

  // Cerrar y eliminar archivo de prueba
  file.close();
  static_cast<void>(std::remove(filename.c_str()));
}

// Test 2: Datos de píxeles con tamaño inválido
TEST_F(PPMWriterTest, InvalidPixelDataSize) {
  // Configuración
  std::string const filename = "test_error.ppm";
  size_t const width         = 5;
  size_t const height        = 5;  // total_pixels = 25

  // Crear estructura Pixels con datos INCORRECTOS (solo 3 píxeles en lugar de 25)
  // Los canales son std::vector<uint8_t> de tamaño 3, no 25
  std::vector<uint8_t> const r_channel = {1, 2, 3};
  std::vector<uint8_t> const g_channel = {1, 2, 3};
  std::vector<uint8_t> const b_channel = {1, 2, 3};

  PPMWriter::Pixels const pixels(r_channel, g_channel, b_channel, width, height);

  // Llamar a write_ppm
  bool const result = PPMWriter::write_ppm(filename, pixels);

  // Verificar que la función retorna false debido al chequeo de tamaño
  ASSERT_FALSE(result) << "write_ppm debería retornar false cuando el tamaño de los canales no "
                          "coincide con width*height";

  // Verificar que no se creó el archivo (o eliminarlo si se creó parcialmente)
  static_cast<void>(std::remove(filename.c_str()));
}

// Test 3: Ruta de archivo inválida
TEST_F(PPMWriterTest, InvalidFilePath) {
  // Configuración: Intentar escribir en un directorio que no existe
  std::string const filename = "invalid_directory/test_error.ppm";
  size_t const width         = 1;
  size_t const height        = 1;

  // Crear estructura Pixels con 1 píxel válido
  // Los canales son std::vector<uint8_t>
  std::vector<uint8_t> const r_channel = {255};
  std::vector<uint8_t> const g_channel = {255};
  std::vector<uint8_t> const b_channel = {255};

  PPMWriter::Pixels const pixels(r_channel, g_channel, b_channel, width, height);

  // Llamar a write_ppm
  bool const result = PPMWriter::write_ppm(filename, pixels);

  // Verificar que la función retorna false debido a que no se pudo abrir el archivo
  ASSERT_FALSE(result) << "write_ppm debería retornar false cuando no se puede abrir el archivo";
}
