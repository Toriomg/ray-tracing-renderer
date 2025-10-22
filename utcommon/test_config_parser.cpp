#include "config_parser.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string_view>
#include <vector>

// Fixture for ConfigParser tests
class ConfigParserTest : public ::testing::Test {
protected:
  ConfigSettings config;

  void SetUp() override {
    // Initialize config with a known value before each test
    // Using -1 as sentinel to detect if the value was modified
    config.image_width = -1;
  }

  void TearDown() override {
    // Clean up after each test if needed
  }
};

// ============================================================================
// TESTS PARA parseImageWidth
// ============================================================================

class ConfigParserImageWidthTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserImageWidthTest, ValidBasicCase) {
  // Test básico: valor válido 1920
  writeConfigFile("image_width: 1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserImageWidthTest, ValidAlternativeValue) {
  // Otro caso válido: valor 1280
  writeConfigFile("image_width: 1280\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'280);
}

TEST_F(ConfigParserImageWidthTest, ValidSmallValue) {
  // Valor pequeño pero válido: 1
  writeConfigFile("image_width: 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1);
}

TEST_F(ConfigParserImageWidthTest, ValidLargeValue) {
  // Valor grande para verificar que no hay límite superior artificial
  // Esto es importante porque algunos sistemas pueden tener restricciones
  // de memoria, pero la función de parsing no debe rechazar valores grandes
  writeConfigFile("image_width: 7680\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 7'680);
}

TEST_F(ConfigParserImageWidthTest, ValidVeryLargeValue) {
  // Valor muy grande (8K) para asegurar robustez
  // Añado este test porque en aplicaciones de renderizado es común
  // trabajar con resoluciones muy altas
  writeConfigFile("image_width: 15360\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 15'360);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserImageWidthTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("image_width:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("image_width: 1920 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("image_width: 1920 1080 720\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserImageWidthTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("image_width: abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("image_width: 1920abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorPartialNumericValue) {
  // Valor con caracteres numéricos y no numéricos
  writeConfigFile("image_width: abc1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorFloatingPointValue) {
  // Valor de punto flotante (debería rechazarse ya que espera int)
  // Este test es importante porque parseInt debe rechazar decimales
  writeConfigFile("image_width: 1920.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserImageWidthTest, ErrorZeroValue) {
  // Valor cero (no positivo)
  writeConfigFile("image_width: 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorNegativeValue) {
  // Valor negativo
  writeConfigFile("image_width: -1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ErrorNegativeSmallValue) {
  // Otro valor negativo pequeño
  writeConfigFile("image_width: -1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserImageWidthTest, ExtraWhitespaceBeforeValue) {
  // Espacios extra antes del valor
  // Este test verifica que trimWhitespace funciona correctamente
  writeConfigFile("image_width:    1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserImageWidthTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  // Verifica el manejo robusto de whitespace
  writeConfigFile("  image_width: 1920  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserImageWidthTest, TabCharacters) {
  // Tabs en lugar de espacios
  // NOTA: El tokenizador actual solo maneja espacios, no tabs.
  // Este test documenta el comportamiento actual donde tabs hacen que
  // el parsing falle. Mantener valor por defecto.
  writeConfigFile("image_width:\t1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El parser actual no maneja tabs, mantiene valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# image_width: 1920\nimage_width: 1280\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'280);
}

TEST_F(ConfigParserImageWidthTest, EmptyLineBetweenCommands) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\nimage_width: 1920\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserImageWidthTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que image_width se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "image_width: 2560\n"
                  "samples_per_pixel: 100\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 2'560);
  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  ASSERT_EQ(config.samples_per_pixel, 100);
}

TEST_F(ConfigParserImageWidthTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("image_width: 1920\n"
                  "image_width: 1280\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'280);
}

TEST_F(ConfigParserImageWidthTest, IntegerOverflowProtection) {
  // Valor que podría causar overflow en int (mayor que INT_MAX)
  // Este test verifica la robustez ante valores extremos
  // INT_MAX típicamente es 2147483647
  writeConfigFile("image_width: 9999999999\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debería fallar el parsing y mantener el valor por defecto
  // porque std::from_chars detectará el overflow
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente sin interpretarse como octal
  writeConfigFile("image_width: 001920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserImageWidthTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: std::from_chars para enteros NO acepta el signo + en esta configuración.
  // Este test documenta que valores con '+' explícito son rechazados.
  writeConfigFile("image_width: +1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El parser rechaza el signo +, mantiene valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

TEST_F(ConfigParserImageWidthTest, ScientificNotation) {
  // Notación científica (no debería ser aceptada para enteros)
  writeConfigFile("image_width: 1e3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.image_width, Constants::ImageWidth);
}

// ============================================================================
// TESTS PARA parseCameraPosition
// ============================================================================

class ConfigParserCameraPositionTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_camera_position_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserCameraPositionTest, ValidBasicCase) {
  // Test básico: posición arbitraria con valores positivos y negativos
  writeConfigFile("camera_position: 10 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, ValidOriginPosition) {
  // Posición en el origen (0, 0, 0)
  writeConfigFile("camera_position: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
}

TEST_F(ConfigParserCameraPositionTest, ValidAllNegativeValues) {
  // Todos los valores negativos
  writeConfigFile("camera_position: -1.5 -2.5 -3.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, -1.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, -2.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -3.5);
}

TEST_F(ConfigParserCameraPositionTest, ValidAllPositiveValues) {
  // Todos los valores positivos
  writeConfigFile("camera_position: 100.5 200.75 300.25\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 100.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 200.75);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, 300.25);
}

TEST_F(ConfigParserCameraPositionTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_position: 10 20 30\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, 30.0);
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotation) {
  // Notación científica: 1e1 = 10.0, 2.0e1 = 20.0, -5.5e0 = -5.5
  writeConfigFile("camera_position: 1e1 2.0e1 -5.5e0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos: 1e-1 = 0.1
  writeConfigFile("camera_position: 1e-1 2.5e-2 -3.3e-3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.1);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.025);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -0.0033);
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponentes grandes
  writeConfigFile("camera_position: 1e5 2e6 -3e7\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 100000.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 2000000.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -30000000.0);
}

TEST_F(ConfigParserCameraPositionTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  writeConfigFile("camera_position: 1000000.5 2000000.75 -3000000.25\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 1000000.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 2000000.75);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -3000000.25);
}

TEST_F(ConfigParserCameraPositionTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero)
  writeConfigFile("camera_position: 0.0001 0.00001 -0.000001\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0001);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.00001);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -0.000001);
}

TEST_F(ConfigParserCameraPositionTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_position: 10 20.5 -3e1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -30.0);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_position:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_position: 10\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_position: 10 20\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_position: 10 20 30 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_position: 10 20 30 40 50\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_position: abc 20 30\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_position: 10 abc 30\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_position: 10 20 abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_position: abc def ghi\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_position: 10abc 20def 30ghi\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_position: abc10 def20 ghi30\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraPositionTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_position:    10    20    -5.5   \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_position: 10 20 -5.5  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_position: 10     20     -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_position: 1 2 3\ncamera_position: 10 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_position: 10 20 -5.5\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_position se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "camera_position: 5.5 10.5 -15.5\n"
                  "image_width: 1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 5.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 10.5);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -15.5);
  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserCameraPositionTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_position: 1 2 3\n"
                  "camera_position: 10 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_position: 0010 0020 -005.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_position: +10 +20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  // Este test verifica la robustez ante valores extremos
  writeConfigFile("camera_position: 1e400 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars debería detectar el overflow y fallar el parsing
  // mantiene el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, DoubleUnderflowToZero) {
  // Valores extremadamente pequeños que underflow a cero
  // Esto debería ser válido
  writeConfigFile("camera_position: 1e-400 1e-400 1e-400\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
}

TEST_F(ConfigParserCameraPositionTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_position: inf 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_position: 10 nan -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_position: 10.5.5 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_position: 10! 20@ -5.5#\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_position: 0x10 0x20 -0x5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
}

TEST_F(ConfigParserCameraPositionTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_position: 10.123456789012345 20.987654321098765 -5.555555555555555\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El valor será parseado con la precisión disponible de double
  ASSERT_NEAR(config.camera_pos.x, 10.123456789012345, 1e-15);
  ASSERT_NEAR(config.camera_pos.y, 20.987654321098765, 1e-15);
  ASSERT_NEAR(config.camera_pos.z, -5.555555555555555, 1e-15);
}

TEST_F(ConfigParserCameraPositionTest, EmptyTokens) {
  // Este test verifica el comportamiento cuando hay tokens "vacíos"
  // debido a múltiples espacios consecutivos que el tokenizer podría manejar
  // Sin embargo, el tokenizer actual debería manejar esto correctamente
  writeConfigFile("camera_position: 10 20 -5.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
}

TEST_F(ConfigParserCameraPositionTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  // Este test documenta el comportamiento con -0.0
  writeConfigFile("camera_position: -0.0 0.0 -0.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // -0.0 y 0.0 son iguales en comparaciones
  ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
}

// ============================================================================
// TESTS PARA parseCameraTarget
// ============================================================================

class ConfigParserCameraTargetTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_camera_target_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserCameraTargetTest, ValidBasicCase) {
  // Test básico: target en el origen (punto común de enfoque)
  writeConfigFile("camera_target: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, ValidAlternativeValue) {
  // Otro caso válido: target con valores mixtos (entero, decimal, científico)
  writeConfigFile("camera_target: 1 -2.5 3.1e1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
}

TEST_F(ConfigParserCameraTargetTest, ValidAllPositiveValues) {
  // Todos los valores positivos
  writeConfigFile("camera_target: 5.5 10.25 15.75\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 5.5);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 10.25);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 15.75);
}

TEST_F(ConfigParserCameraTargetTest, ValidAllNegativeValues) {
  // Todos los valores negativos
  writeConfigFile("camera_target: -3.5 -7.5 -11.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, -3.5);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -7.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, -11.5);
}

TEST_F(ConfigParserCameraTargetTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_target: 5 10 15\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 5.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 15.0);
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotation) {
  // Notación científica: 1e1 = 10.0, 2e0 = 2.0, 3e-1 = 0.3
  writeConfigFile("camera_target: 1e1 2e0 3e-1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 2.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.3);
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos
  writeConfigFile("camera_target: 5e-2 1.5e-1 -2.2e-3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.05);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.15);
  ASSERT_DOUBLE_EQ(config.camera_target.z, -0.0022);
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponentes grandes
  writeConfigFile("camera_target: 2e3 -3e4 4.5e5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 2000.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -30000.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 450000.0);
}

TEST_F(ConfigParserCameraTargetTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  // Importante para escenas con objetos distantes
  writeConfigFile("camera_target: 500000.5 1000000.25 -750000.75\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 500000.5);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 1000000.25);
  ASSERT_DOUBLE_EQ(config.camera_target.z, -750000.75);
}

TEST_F(ConfigParserCameraTargetTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero)
  // Útil para targets precisos cerca del origen
  writeConfigFile("camera_target: 0.0005 -0.00025 0.000125\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0005);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -0.00025);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.000125);
}

TEST_F(ConfigParserCameraTargetTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_target: 5 -7.25 1.5e2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 5.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -7.25);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 150.0);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_target:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_target: 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_target: 1 -2.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_target: 0 0 0 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_target: 0 0 0 1 2 3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_target: abc 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_target: 0 abc 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_target: 0 0 abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_target: foo bar baz\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_target: 1abc -2.5def 3e1ghi\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_target: abc1 def2.5 ghi3e1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraTargetTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_target:    1    -2.5    3.1e1   \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
}

TEST_F(ConfigParserCameraTargetTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_target: 0 0 0  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_target: 0     0     0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_target: 10 20 30\ncamera_target: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_target: 1 -2.5 31\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
}

TEST_F(ConfigParserCameraTargetTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_target se procesa correctamente en un contexto más amplio
  writeConfigFile("camera_position: 10 20 -5.5\n"
                  "camera_target: 0 0 0\n"
                  "gamma: 2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserCameraTargetTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_target: 5 10 15\n"
                  "camera_target: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_target: 001 -002.5 0031\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
}

TEST_F(ConfigParserCameraTargetTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_target: +1 +2.5 +3.1e1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 2.5);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
}

TEST_F(ConfigParserCameraTargetTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  writeConfigFile("camera_target: 1e400 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars debería detectar el overflow y fallar el parsing
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, DoubleUnderflowToZero) {
  // Valores extremadamente pequeños que underflow a cero
  writeConfigFile("camera_target: 1e-400 1e-400 1e-400\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  // Este test documenta el comportamiento con valores infinitos como string
  writeConfigFile("camera_target: inf 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_target: 0 nan 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_target: 1.2.3 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_target: 1! -2.5@ 3#\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_target: 0x1 0x2 0x3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
  ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
  ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
}

TEST_F(ConfigParserCameraTargetTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_target: 1.123456789012345 -2.987654321098765 31.555555555555555\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El valor será parseado con la precisión disponible de double
  ASSERT_NEAR(config.camera_target.x, 1.123456789012345, 1e-15);
  ASSERT_NEAR(config.camera_target.y, -2.987654321098765, 1e-15);
  ASSERT_NEAR(config.camera_target.z, 31.555555555555555, 1e-15);
}

TEST_F(ConfigParserCameraTargetTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  writeConfigFile("camera_target: -0.0 -0.0 0.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // -0.0 y 0.0 son iguales en comparaciones
  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

TEST_F(ConfigParserCameraTargetTest, CombinedWithCameraPosition) {
  // Test de integración: verifica que camera_target y camera_position
  // se pueden usar juntos correctamente, lo cual es común en una configuración real
  writeConfigFile("camera_position: 10 0 -10\n"
                  "camera_target: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Verifica ambos parámetros
  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -10.0);
  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
}

// ============================================================================
// TESTS PARA parseCameraNorth
// ============================================================================

class ConfigParserCameraNorthTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_camera_north_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserCameraNorthTest, ValidBasicCase) {
  // Test básico: vector "up" común (0, 1, 0) - eje Y positivo
  writeConfigFile("camera_north: 0 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidAlternativeVector) {
  // Otro vector válido: diagonal normalizado parcialmente
  writeConfigFile("camera_north: 0.5 0.5 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidZAxisUp) {
  // Vector "up" en Z positivo (común en algunas convenciones)
  writeConfigFile("camera_north: 0 0 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 1.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidNegativeYAxis) {
  // Vector "up" negativo (cámara invertida)
  writeConfigFile("camera_north: 0 -1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, -1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidAllPositiveValues) {
  // Todos los componentes positivos
  writeConfigFile("camera_north: 0.577 0.577 0.577\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.577);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.577);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.577);
}

TEST_F(ConfigParserCameraNorthTest, ValidAllNegativeValues) {
  // Todos los componentes negativos
  writeConfigFile("camera_north: -0.5 -0.5 -0.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, -0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.y, -0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.z, -0.5);
}

TEST_F(ConfigParserCameraNorthTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_north: 1 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotation) {
  // Notación científica: 5e-1 = 0.5, 5e-1 = 0.5, 0e0 = 0.0
  writeConfigFile("camera_north: 5e-1 5e-1 0e0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.5);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos
  writeConfigFile("camera_north: 1e-1 2e-1 3e-1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.1);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.2);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.3);
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotationPositiveExponent) {
  // Notación científica con exponentes positivos (valores grandes)
  // Aunque no es típico para vectores de dirección, el parser lo acepta
  writeConfigFile("camera_north: 1e2 2e2 3e2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 100.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 200.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 300.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidZeroVector) {
  // Vector cero (0, 0, 0) - técnicamente válido para el parser
  // aunque podría causar problemas matemáticos más adelante (normalización)
  // Este test documenta que el parser NO valida la magnitud del vector
  writeConfigFile("camera_north: 0 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  // No típico para vectores de dirección pero el parser lo acepta
  writeConfigFile("camera_north: 1000.5 2000.25 3000.75\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 1000.5);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 2000.25);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 3000.75);
}

TEST_F(ConfigParserCameraNorthTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero pero no cero)
  writeConfigFile("camera_north: 0.0001 0.0002 0.0003\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0001);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0002);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0003);
}

TEST_F(ConfigParserCameraNorthTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_north: 0 1.0 0e0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_north:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_north: 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_north: 0 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_north: 0 1 0 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_north: 0 1 0 1 2 3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_north: abc 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_north: 0 abc 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_north: 0 1 abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_north: up vector here\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_north: 0abc 1def 0ghi\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_north: abc0 def1 ghi0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraNorthTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_north:    0    1    0   \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_north: 0 1 0  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_north: 0     1     0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_north: 1 0 0\ncamera_north: 0 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_north: 0 1 0\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_north se procesa correctamente en un contexto más amplio
  writeConfigFile("camera_position: 10 20 -5.5\n"
                  "camera_target: 0 0 0\n"
                  "camera_north: 0 1 0\n"
                  "gamma: 2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserCameraNorthTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_north: 1 0 0\n"
                  "camera_north: 0 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_north: 00 01 00\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_north: +0 +1 +0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  writeConfigFile("camera_north: 1e400 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars debería detectar el overflow y fallar el parsing
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, DoubleUnderflowToZero) {
  // Valores extremadamente pequeños que underflow a cero
  writeConfigFile("camera_north: 1e-400 1e-400 1e-400\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  // Este test documenta el comportamiento con valores infinitos como string
  writeConfigFile("camera_north: inf 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_north: 0 nan 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_north: 0.0.0 1 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_north: 0! 1@ 0#\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_north: 0x0 0x1 0x0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
  ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
  ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
}

TEST_F(ConfigParserCameraNorthTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_north: 0.123456789012345 1.987654321098765 0.555555555555555\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El valor será parseado con la precisión disponible de double
  ASSERT_NEAR(config.camera_north.x, 0.123456789012345, 1e-15);
  ASSERT_NEAR(config.camera_north.y, 1.987654321098765, 1e-15);
  ASSERT_NEAR(config.camera_north.z, 0.555555555555555, 1e-15);
}

TEST_F(ConfigParserCameraNorthTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  writeConfigFile("camera_north: -0.0 1.0 -0.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // -0.0 y 0.0 son iguales en comparaciones
  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
}

TEST_F(ConfigParserCameraNorthTest, NormalizedVector) {
  // Vector normalizado típico (sqrt(1/3) en cada componente)
  // Este test documenta que el parser NO normaliza el vector automáticamente
  // La normalización debe hacerse en otro lugar del código si es necesaria
  writeConfigFile("camera_north: 0.577350269189626 0.577350269189626 0.577350269189626\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_NEAR(config.camera_north.x, 0.577350269189626, 1e-15);
  ASSERT_NEAR(config.camera_north.y, 0.577350269189626, 1e-15);
  ASSERT_NEAR(config.camera_north.z, 0.577350269189626, 1e-15);
}

TEST_F(ConfigParserCameraNorthTest, CombinedCameraParameters) {
  // Test de integración: verifica que todos los parámetros de cámara
  // se pueden usar juntos correctamente
  writeConfigFile("camera_position: 10 5 -10\n"
                  "camera_target: 0 0 0\n"
                  "camera_north: 0 1 0\n"
                  "field_of_view: 90\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Verifica todos los parámetros de cámara
  ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.y, 5.0);
  ASSERT_DOUBLE_EQ(config.camera_pos.z, -10.0);
  ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
  ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
  ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
}

// ============================================================================
// TESTS PARA parseAspectRatio
// ============================================================================

class ConfigParserAspectRatioTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_aspect_ratio_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserAspectRatioTest, ValidBasicCase) {
  // Test básico: aspect ratio común 16:9
  writeConfigFile("aspect_ratio: 16 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, ValidAlternativeRatio) {
  // Otro caso válido: aspect ratio 4:3 (común en monitores antiguos)
  writeConfigFile("aspect_ratio: 4 3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 4U);
  ASSERT_EQ(config.aspect_ratio.second, 3U);
}

TEST_F(ConfigParserAspectRatioTest, ValidWideScreenRatio) {
  // Aspect ratio ultrawide 21:9
  writeConfigFile("aspect_ratio: 21 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 21U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, ValidSquareRatio) {
  // Aspect ratio cuadrado 1:1
  writeConfigFile("aspect_ratio: 1 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 1U);
  ASSERT_EQ(config.aspect_ratio.second, 1U);
}

TEST_F(ConfigParserAspectRatioTest, ValidCinematicRatio) {
  // Aspect ratio cinematográfico 2.39:1 representado como 239:100
  // Este test verifica que valores más grandes funcionan correctamente
  writeConfigFile("aspect_ratio: 239 100\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 239U);
  ASSERT_EQ(config.aspect_ratio.second, 100U);
}

TEST_F(ConfigParserAspectRatioTest, ValidLargeValues) {
  // Valores grandes pero válidos para verificar robustez
  // Útil para verificar que no hay límites artificiales
  writeConfigFile("aspect_ratio: 3840 2160\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 3'840U);
  ASSERT_EQ(config.aspect_ratio.second, 2'160U);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserAspectRatioTest, ErrorTooFewArguments_MissingBoth) {
  // Menos de 3 tokens: faltan ambos valores
  writeConfigFile("aspect_ratio:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooFewArguments_MissingHeight) {
  // Menos de 3 tokens: falta el segundo valor (height)
  writeConfigFile("aspect_ratio: 16\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooManyArguments_OneExtra) {
  // Más de 3 tokens: un argumento extra
  writeConfigFile("aspect_ratio: 16 9 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooManyArguments_Multiple) {
  // Más de 3 tokens: múltiples argumentos extra
  writeConfigFile("aspect_ratio: 16 9 4 3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserAspectRatioTest, ErrorNonNumericWidth) {
  // Primer valor no numérico
  writeConfigFile("aspect_ratio: abc 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorNonNumericHeight) {
  // Segundo valor no numérico
  writeConfigFile("aspect_ratio: 16 abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothNonNumeric) {
  // Ambos valores no numéricos
  writeConfigFile("aspect_ratio: abc def\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("aspect_ratio: 16abc 9def\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorFloatingPointValues) {
  // Valores de punto flotante (deberían rechazarse ya que espera unsigned int)
  // Este test es importante porque parseUnsignedInt debe rechazar decimales
  writeConfigFile("aspect_ratio: 16.5 9.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserAspectRatioTest, ErrorZeroWidth) {
  // Width es cero (no positivo)
  writeConfigFile("aspect_ratio: 0 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorZeroHeight) {
  // Height es cero (no positivo)
  writeConfigFile("aspect_ratio: 16 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothZero) {
  // Ambos valores cero
  writeConfigFile("aspect_ratio: 0 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorNegativeWidth) {
  // Width negativo (debería ser rechazado por parseUnsignedInt)
  // parseUnsignedInt NO acepta valores negativos
  writeConfigFile("aspect_ratio: -16 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorNegativeHeight) {
  // Height negativo (debería ser rechazado por parseUnsignedInt)
  writeConfigFile("aspect_ratio: 16 -9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothNegative) {
  // Ambos valores negativos
  writeConfigFile("aspect_ratio: -16 -9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserAspectRatioTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("aspect_ratio:    16    9   \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  aspect_ratio: 16 9  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  // El tokenizer actual debería manejar esto correctamente
  writeConfigFile("aspect_ratio: 16     9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# aspect_ratio: 4 3\naspect_ratio: 16 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\naspect_ratio: 16 9\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que aspect_ratio se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "aspect_ratio: 21 9\n"
                  "image_width: 2560\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 21U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  ASSERT_EQ(config.image_width, 2'560);
}

TEST_F(ConfigParserAspectRatioTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("aspect_ratio: 16 9\n"
                  "aspect_ratio: 4 3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 4U);
  ASSERT_EQ(config.aspect_ratio.second, 3U);
}

TEST_F(ConfigParserAspectRatioTest, UnsignedIntMaxValue) {
  // Valor máximo para unsigned int (típicamente 4294967295)
  // Este test verifica que valores muy grandes pero válidos funcionan
  writeConfigFile("aspect_ratio: 4294967295 1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 4'294'967'295U);
  ASSERT_EQ(config.aspect_ratio.second, 1U);
}

TEST_F(ConfigParserAspectRatioTest, UnsignedIntOverflow) {
  // Valor que excede UINT_MAX (debería causar overflow)
  // Este test verifica la robustez ante valores extremos
  // UINT_MAX típicamente es 4294967295
  writeConfigFile("aspect_ratio: 4294967296 9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debería fallar el parsing y mantener el valor por defecto
  // porque std::from_chars detectará el overflow
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente sin interpretarse como octal
  writeConfigFile("aspect_ratio: 0016 009\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
}

TEST_F(ConfigParserAspectRatioTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: std::from_chars para enteros sin signo NO acepta el signo +.
  // Este test documenta que valores con '+' explícito son rechazados.
  writeConfigFile("aspect_ratio: +16 +9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El parser rechaza el signo +, mantiene valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ScientificNotation) {
  // Notación científica (no debería ser aceptada para enteros sin signo)
  writeConfigFile("aspect_ratio: 1e2 9e0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada sin configuración especial)
  // std::from_chars en base 10 (por defecto) no acepta prefijo 0x
  writeConfigFile("aspect_ratio: 0x10 0x09\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  // Este test verifica que el parser es robusto ante entradas malformadas
  writeConfigFile("aspect_ratio: 16! 9@\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ColonInsteadOfSpace) {
  // Usar ':' en lugar de espacio (error común de formato)
  // El tokenizer actual busca espacios, no dos puntos
  writeConfigFile("aspect_ratio: 16:9\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto (parsing fallará por formato incorrecto)
  ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
  ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
}

TEST_F(ConfigParserAspectRatioTest, ExtremelyLargeValidValues) {
  // Valores grandes pero válidos para verificar el límite superior
  // Útil para aplicaciones que renderizan a resoluciones muy altas
  writeConfigFile("aspect_ratio: 1000000 562500\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_EQ(config.aspect_ratio.first, 1'000'000U);
  ASSERT_EQ(config.aspect_ratio.second, 562'500U);
}

// ============================================================================
// TESTS PARA parseGamma
// ============================================================================

class ConfigParserGammaTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_gamma_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeConfigFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(ConfigParserGammaTest, ValidBasicCase) {
  // Test básico: gamma común 2.2
  writeConfigFile("gamma: 2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, ValidAlternativeValue) {
  // Otro valor válido: gamma 1.8
  writeConfigFile("gamma: 1.8\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 1.8);
}

TEST_F(ConfigParserGammaTest, ValidIntegerValue) {
  // Valor entero: gamma 2 (debería convertirse a 2.0)
  writeConfigFile("gamma: 2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.0);
}

TEST_F(ConfigParserGammaTest, ValidScientificNotation) {
  // Notación científica: 1e-1 = 0.1
  // Este test verifica que parsedouble acepta notación científica
  writeConfigFile("gamma: 1e-1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.1);
}

TEST_F(ConfigParserGammaTest, ValidScientificNotationPositiveExponent) {
  // Notación científica con exponente positivo: 2.2e0 = 2.2
  writeConfigFile("gamma: 2.2e0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponente mayor: 1.5e2 = 150.0
  writeConfigFile("gamma: 1.5e2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 150.0);
}

TEST_F(ConfigParserGammaTest, ValidZeroValue) {
  // Valor cero (técnicamente válido aunque poco práctico para gamma)
  // parseGamma no valida rangos, solo acepta doubles válidos
  writeConfigFile("gamma: 0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.0);
}

TEST_F(ConfigParserGammaTest, ValidZeroDecimal) {
  // Valor cero con decimales
  writeConfigFile("gamma: 0.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.0);
}

TEST_F(ConfigParserGammaTest, ValidNegativeValue) {
  // Valor negativo (técnicamente válido según parseGamma, no hay validación de rango)
  writeConfigFile("gamma: -1.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, -1.0);
}

TEST_F(ConfigParserGammaTest, ValidSmallPositiveValue) {
  // Valor pequeño positivo
  writeConfigFile("gamma: 0.001\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.001);
}

TEST_F(ConfigParserGammaTest, ValidLargeValue) {
  // Valor grande pero válido
  writeConfigFile("gamma: 100.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 100.5);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserGammaTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("gamma:\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("gamma: 2.2 extra\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("gamma: 2.2 1.8 3.0\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserGammaTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("gamma: abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("gamma: 2.2abc\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ErrorPartialNumericValue) {
  // Valor con caracteres numéricos y no numéricos al inicio
  writeConfigFile("gamma: abc2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ErrorEmptyValue) {
  // Token vacío (debería ser detectado por parsedouble)
  writeConfigFile("gamma: \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserGammaTest, ExtraWhitespaceAroundValue) {
  // Espacios extra alrededor del valor
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("gamma:    2.2   \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  gamma: 2.2  \n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# gamma: 1.0\ngamma: 2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ngamma: 2.2\n\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que gamma se procesa correctamente en un contexto más amplio
  writeConfigFile("aspect_ratio: 16 9\n"
                  "gamma: 1.8\n"
                  "image_width: 1920\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 1.8);
  ASSERT_EQ(config.aspect_ratio.first, 16U);
  ASSERT_EQ(config.aspect_ratio.second, 9U);
  ASSERT_EQ(config.image_width, 1'920);
}

TEST_F(ConfigParserGammaTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("gamma: 2.2\n"
                  "gamma: 1.8\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 1.8);
}

TEST_F(ConfigParserGammaTest, VeryLargeValidValue) {
  // Valor muy grande pero válido para double
  // Este test verifica que no hay límites artificiales
  writeConfigFile("gamma: 1000000.5\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 1000000.5);
}

TEST_F(ConfigParserGammaTest, VerySmallValidValue) {
  // Valor muy pequeño pero válido
  writeConfigFile("gamma: 0.0000001\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.0000001);
}

TEST_F(ConfigParserGammaTest, ScientificNotationNegativeExponent) {
  // Notación científica con exponente negativo: 2.2e-3 = 0.0022
  writeConfigFile("gamma: 2.2e-3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 0.0022);
}

TEST_F(ConfigParserGammaTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente
  writeConfigFile("gamma: 002.200\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, PlusSignPrefix) {
  // Signo + explícito
  // std::from_chars para doubles SÍ acepta el signo +
  writeConfigFile("gamma: +2.2\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  ASSERT_DOUBLE_EQ(config.gamma, 2.2);
}

TEST_F(ConfigParserGammaTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double
  // Este test verifica la robustez ante valores extremos
  // Un valor mayor que DBL_MAX debería ser manejado por std::from_chars
  writeConfigFile("gamma: 1e400\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars debería detectar el overflow y fallar el parsing
  // mantiene el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, DoubleUnderflowToZero) {
  // Valor extremadamente pequeño que podría underflow a cero
  // Esto debería ser válido ya que parsedouble lo acepta
  writeConfigFile("gamma: 1e-400\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debería ser parseado como 0.0 (underflow)
  ASSERT_DOUBLE_EQ(config.gamma, 0.0);
}

TEST_F(ConfigParserGammaTest, InvalidInfinityString) {
  // String "inf" - std::from_chars puede o no aceptar esto
  // Este test documenta el comportamiento actual
  writeConfigFile("gamma: inf\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars típicamente NO acepta "inf" como string
  // mantiene el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, InvalidNaNString) {
  // String "nan" - std::from_chars puede o no aceptar esto
  // Este test documenta el comportamiento actual
  writeConfigFile("gamma: nan\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // std::from_chars típicamente NO acepta "nan" como string
  // mantiene el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("gamma: 2.2.3\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, SpecialCharactersInValue) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("gamma: 2.2!\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada por std::from_chars en modo decimal)
  writeConfigFile("gamma: 0x1.8p1\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // Debe mantener el valor por defecto
  ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
}

TEST_F(ConfigParserGammaTest, ExtremelyLongDecimal) {
  // Número con muchísimos decimales para verificar precisión
  // Este test es importante porque verifica que parsedouble maneja
  // números con alta precisión correctamente
  writeConfigFile("gamma: 2.2222222222222222222222222222\n");

  ConfigSettings config = loadConfigFromFile(temp_filename);

  // El valor será parseado con la precisión disponible de double
  // Verificamos que es aproximadamente correcto
  ASSERT_NEAR(config.gamma, 2.2222222222222222, 1e-15);
}

// ============================================================================
// Main para ejecutar los tests
// ============================================================================

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
