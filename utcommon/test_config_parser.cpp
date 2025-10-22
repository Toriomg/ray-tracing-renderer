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

// Since parseImageWidth is in an anonymous namespace, we test it indirectly
// through file parsing. We'll create temporary config files for testing.

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

// ============================================================================
// CASOS VÁLIDOS
// ============================================================================

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

// ============================================================================
// ERRORES DE FORMATO - Número incorrecto de argumentos
// ============================================================================

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

// ============================================================================
// ERRORES DE FORMATO - Valores no numéricos
// ============================================================================

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

// ============================================================================
// ERRORES DE RANGO - Valores no positivos
// ============================================================================

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

// ============================================================================
// CASOS ADICIONALES - Edge cases y robustez
// ============================================================================

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
// Main para ejecutar los tests
// ============================================================================

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
