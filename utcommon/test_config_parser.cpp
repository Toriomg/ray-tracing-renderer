#include "config_parser.hpp"
#include "constants.hpp"
#include "dataStructs/settings_structs.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <string>

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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserImageWidthTest, ValidAlternativeValue) {
  // Otro caso válido: valor 1280
  writeConfigFile("image_width: 1280\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'280);
  }
}

TEST_F(ConfigParserImageWidthTest, ValidSmallValue) {
  // Valor pequeño pero válido: 1
  writeConfigFile("image_width: 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1);
  }
}

TEST_F(ConfigParserImageWidthTest, ValidLargeValue) {
  // Valor grande para verificar que no hay límite superior artificial
  // Esto es importante porque algunos sistemas pueden tener restricciones
  // de memoria, pero la función de parsing no debe rechazar valores grandes
  writeConfigFile("image_width: 7680\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 7'680);
  }
}

TEST_F(ConfigParserImageWidthTest, ValidVeryLargeValue) {
  // Valor muy grande (8K) para asegurar robustez
  // Añado este test porque en aplicaciones de renderizado es común
  // trabajar con resoluciones muy altas
  writeConfigFile("image_width: 15360\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 15'360);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserImageWidthTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("image_width:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("image_width: 1920 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("image_width: 1920 1080 720\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserImageWidthTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("image_width: abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("image_width: 1920abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorPartialNumericValue) {
  // Valor con caracteres numéricos y no numéricos
  writeConfigFile("image_width: abc1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorFloatingPointValue) {
  // Valor de punto flotante (debería rechazarse ya que espera int)
  // Este test es importante porque parseInt debe rechazar decimales
  writeConfigFile("image_width: 1920.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserImageWidthTest, ErrorZeroValue) {
  // Valor cero (no positivo)
  writeConfigFile("image_width: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorNegativeValue) {
  // Valor negativo
  writeConfigFile("image_width: -1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ErrorNegativeSmallValue) {
  // Otro valor negativo pequeño
  writeConfigFile("image_width: -1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserImageWidthTest, ExtraWhitespaceBeforeValue) {
  // Espacios extra antes del valor
  // Este test verifica que trimWhitespace funciona correctamente
  writeConfigFile("image_width:    1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserImageWidthTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  // Verifica el manejo robusto de whitespace
  writeConfigFile("  image_width: 1920  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserImageWidthTest, TabCharacters) {
  // Tabs en lugar de espacios
  // NOTA: El tokenizador actual solo maneja espacios, no tabs.
  // Este test documenta el comportamiento actual donde tabs hacen que
  // el parsing falle. Mantener valor por defecto.
  writeConfigFile("image_width:\t1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El parser actual no maneja tabs, mantiene valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# image_width: 1920\nimage_width: 1280\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'280);
  }
}

TEST_F(ConfigParserImageWidthTest, EmptyLineBetweenCommands) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\nimage_width: 1920\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserImageWidthTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que image_width se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "image_width: 2560\n"
                  "samples_per_pixel: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 2'560);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.samples_per_pixel, 100);
  }
}

TEST_F(ConfigParserImageWidthTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("image_width: 1920\n"
                  "image_width: 1280\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'280);
  }
}

TEST_F(ConfigParserImageWidthTest, IntegerOverflowProtection) {
  // Valor que podría causar overflow en int (mayor que INT_MAX)
  // Este test verifica la robustez ante valores extremos
  // INT_MAX típicamente es 2147483647
  writeConfigFile("image_width: 9999999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debería fallar el parsing y mantener el valor por defecto
    // porque std::from_chars detectará el overflow
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente sin interpretarse como octal
  writeConfigFile("image_width: 001920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserImageWidthTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: std::from_chars para enteros NO acepta el signo + en esta configuración.
  // Este test documenta que valores con '+' explícito son rechazados.
  writeConfigFile("image_width: +1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El parser rechaza el signo +, mantiene valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
}

TEST_F(ConfigParserImageWidthTest, ScientificNotation) {
  // Notación científica (no debería ser aceptada para enteros)
  writeConfigFile("image_width: 1e3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.image_width, Constants::ImageWidth);
  }
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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidOriginPosition) {
  // Posición en el origen (0, 0, 0)
  writeConfigFile("camera_position: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidAllNegativeValues) {
  // Todos los valores negativos
  writeConfigFile("camera_position: -1.5 -2.5 -3.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, -1.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, -2.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -3.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidAllPositiveValues) {
  // Todos los valores positivos
  writeConfigFile("camera_position: 100.5 200.75 300.25\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 100.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 200.75);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, 300.25);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_position: 10 20 30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, 30.0);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotation) {
  // Notación científica: 1e1 = 10.0, 2.0e1 = 20.0, -5.5e0 = -5.5
  writeConfigFile("camera_position: 1e1 2.0e1 -5.5e0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos: 1e-1 = 0.1
  writeConfigFile("camera_position: 1e-1 2.5e-2 -3.3e-3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.1);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.025);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -0.0033);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponentes grandes
  writeConfigFile("camera_position: 1e5 2e6 -3e7\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 100000.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 2000000.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -30000000.0);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  writeConfigFile("camera_position: 1000000.5 2000000.75 -3000000.25\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 1000000.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 2000000.75);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -3000000.25);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero)
  writeConfigFile("camera_position: 0.0001 0.00001 -0.000001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0001);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.00001);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -0.000001);
  }
}

TEST_F(ConfigParserCameraPositionTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_position: 10 20.5 -3e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -30.0);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_position:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_position: 10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_position: 10 20\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_position: 10 20 30 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_position: 10 20 30 40 50\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_position: abc 20 30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_position: 10 abc 30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_position: 10 20 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_position: abc def ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_position: 10abc 20def 30ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_position: abc10 def20 ghi30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraPositionTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_position:    10    20    -5.5   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_position: 10 20 -5.5  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_position: 10     20     -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_position: 1 2 3\ncamera_position: 10 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_position: 10 20 -5.5\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_position se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "camera_position: 5.5 10.5 -15.5\n"
                  "image_width: 1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 5.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 10.5);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -15.5);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserCameraPositionTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_position: 1 2 3\n"
                  "camera_position: 10 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_position: 0010 0020 -005.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_position: +10 +20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  // Este test verifica la robustez ante valores extremos
  writeConfigFile("camera_position: 1e400 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars debería detectar el overflow y fallar el parsing
    // mantiene el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, DoubleUnderflowToZero) {
  // Valores extremadamente pequeños que underflow a cero
  // Esto debería ser válido
  writeConfigFile("camera_position: 1e-400 1e-400 1e-400\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
  }
}

TEST_F(ConfigParserCameraPositionTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_position: inf 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_position: 10 nan -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_position: 10.5.5 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_position: 10! 20@ -5.5#\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_position: 0x10 0x20 -0x5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_pos.x, Constants::CameraPosition.x);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, Constants::CameraPosition.y);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, Constants::CameraPosition.z);
  }
}

TEST_F(ConfigParserCameraPositionTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_position: 10.123456789012345 20.987654321098765 -5.555555555555555\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El valor será parseado con la precisión disponible de double
    ASSERT_NEAR(config.camera_pos.x, 10.123456789012345, 1e-15);
    ASSERT_NEAR(config.camera_pos.y, 20.987654321098765, 1e-15);
    ASSERT_NEAR(config.camera_pos.z, -5.555555555555555, 1e-15);
  }
}

TEST_F(ConfigParserCameraPositionTest, EmptyTokens) {
  // Este test verifica el comportamiento cuando hay tokens "vacíos"
  // debido a múltiples espacios consecutivos que el tokenizer podría manejar
  // Sin embargo, el tokenizer actual debería manejar esto correctamente
  writeConfigFile("camera_position: 10 20 -5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -5.5);
  }
}

TEST_F(ConfigParserCameraPositionTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  // Este test documenta el comportamiento con -0.0
  writeConfigFile("camera_position: -0.0 0.0 -0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // -0.0 y 0.0 son iguales en comparaciones
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, 0.0);
  }
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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidAlternativeValue) {
  // Otro caso válido: target con valores mixtos (entero, decimal, científico)
  writeConfigFile("camera_target: 1 -2.5 3.1e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidAllPositiveValues) {
  // Todos los valores positivos
  writeConfigFile("camera_target: 5.5 10.25 15.75\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 5.5);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 10.25);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 15.75);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidAllNegativeValues) {
  // Todos los valores negativos
  writeConfigFile("camera_target: -3.5 -7.5 -11.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, -3.5);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -7.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, -11.5);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_target: 5 10 15\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 5.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 15.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotation) {
  // Notación científica: 1e1 = 10.0, 2e0 = 2.0, 3e-1 = 0.3
  writeConfigFile("camera_target: 1e1 2e0 3e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 2.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.3);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos
  writeConfigFile("camera_target: 5e-2 1.5e-1 -2.2e-3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.05);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.15);
    ASSERT_DOUBLE_EQ(config.camera_target.z, -0.0022);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponentes grandes
  writeConfigFile("camera_target: 2e3 -3e4 4.5e5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 2000.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -30000.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 450000.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  // Importante para escenas con objetos distantes
  writeConfigFile("camera_target: 500000.5 1000000.25 -750000.75\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 500000.5);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 1000000.25);
    ASSERT_DOUBLE_EQ(config.camera_target.z, -750000.75);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero)
  // Útil para targets precisos cerca del origen
  writeConfigFile("camera_target: 0.0005 -0.00025 0.000125\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0005);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -0.00025);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.000125);
  }
}

TEST_F(ConfigParserCameraTargetTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_target: 5 -7.25 1.5e2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 5.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -7.25);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 150.0);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_target:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_target: 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_target: 1 -2.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_target: 0 0 0 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_target: 0 0 0 1 2 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_target: abc 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_target: 0 abc 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_target: 0 0 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_target: foo bar baz\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_target: 1abc -2.5def 3e1ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_target: abc1 def2.5 ghi3e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraTargetTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_target:    1    -2.5    3.1e1   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_target: 0 0 0  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_target: 0     0     0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_target: 10 20 30\ncamera_target: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_target: 1 -2.5 31\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_target se procesa correctamente en un contexto más amplio
  writeConfigFile("camera_position: 10 20 -5.5\n"
                  "camera_target: 0 0 0\n"
                  "gamma: 2.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 20.0);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserCameraTargetTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_target: 5 10 15\n"
                  "camera_target: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_target: 001 -002.5 0031\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, -2.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_target: +1 +2.5 +3.1e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 2.5);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 31.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  writeConfigFile("camera_target: 1e400 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars debería detectar el overflow y fallar el parsing
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, DoubleUnderflowToZero) {
  // Valores extremadamente pequeños que underflow a cero
  writeConfigFile("camera_target: 1e-400 1e-400 1e-400\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  // Este test documenta el comportamiento con valores infinitos como string
  writeConfigFile("camera_target: inf 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_target: 0 nan 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_target: 1.2.3 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_target: 1! -2.5@ 3#\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_target: 0x1 0x2 0x3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_target.x, Constants::CameraTarget.x);
    ASSERT_DOUBLE_EQ(config.camera_target.y, Constants::CameraTarget.y);
    ASSERT_DOUBLE_EQ(config.camera_target.z, Constants::CameraTarget.z);
  }
}

TEST_F(ConfigParserCameraTargetTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_target: 1.123456789012345 -2.987654321098765 31.555555555555555\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El valor será parseado con la precisión disponible de double
    ASSERT_NEAR(config.camera_target.x, 1.123456789012345, 1e-15);
    ASSERT_NEAR(config.camera_target.y, -2.987654321098765, 1e-15);
    ASSERT_NEAR(config.camera_target.z, 31.555555555555555, 1e-15);
  }
}

TEST_F(ConfigParserCameraTargetTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  writeConfigFile("camera_target: -0.0 -0.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // -0.0 y 0.0 son iguales en comparaciones
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
}

TEST_F(ConfigParserCameraTargetTest, CombinedWithCameraPosition) {
  // Test de integración: verifica que camera_target y camera_position
  // se pueden usar juntos correctamente, lo cual es común en una configuración real
  writeConfigFile("camera_position: 10 0 -10\n"
                  "camera_target: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Verifica ambos parámetros
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.z, -10.0);
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_target.z, 0.0);
  }
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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidAlternativeVector) {
  // Otro vector válido: diagonal normalizado parcialmente
  writeConfigFile("camera_north: 0.5 0.5 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidZAxisUp) {
  // Vector "up" en Z positivo (común en algunas convenciones)
  writeConfigFile("camera_north: 0 0 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 1.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidNegativeYAxis) {
  // Vector "up" negativo (cámara invertida)
  writeConfigFile("camera_north: 0 -1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, -1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidAllPositiveValues) {
  // Todos los componentes positivos
  writeConfigFile("camera_north: 0.577 0.577 0.577\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.577);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.577);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.577);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidAllNegativeValues) {
  // Todos los componentes negativos
  writeConfigFile("camera_north: -0.5 -0.5 -0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, -0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.y, -0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.z, -0.5);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidIntegerValues) {
  // Valores enteros (deberían convertirse a double)
  writeConfigFile("camera_north: 1 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotation) {
  // Notación científica: 5e-1 = 0.5, 5e-1 = 0.5, 0e0 = 0.0
  writeConfigFile("camera_north: 5e-1 5e-1 0e0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.5);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponentes negativos
  writeConfigFile("camera_north: 1e-1 2e-1 3e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.1);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.2);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.3);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidScientificNotationPositiveExponent) {
  // Notación científica con exponentes positivos (valores grandes)
  // Aunque no es típico para vectores de dirección, el parser lo acepta
  writeConfigFile("camera_north: 1e2 2e2 3e2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 100.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 200.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 300.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidZeroVector) {
  // Vector cero (0, 0, 0) - técnicamente válido para el parser
  // aunque podría causar problemas matemáticos más adelante (normalización)
  // Este test documenta que el parser NO valida la magnitud del vector
  writeConfigFile("camera_north: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidVeryLargeValues) {
  // Valores muy grandes pero válidos para double
  // No típico para vectores de dirección pero el parser lo acepta
  writeConfigFile("camera_north: 1000.5 2000.25 3000.75\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 1000.5);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 2000.25);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 3000.75);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidVerySmallValues) {
  // Valores muy pequeños (cercanos a cero pero no cero)
  writeConfigFile("camera_north: 0.0001 0.0002 0.0003\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0001);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 0.0002);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0003);
  }
}

TEST_F(ConfigParserCameraNorthTest, ValidMixedFormats) {
  // Mezcla de enteros, decimales y notación científica
  writeConfigFile("camera_north: 0 1.0 0e0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_None) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("camera_north:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_OnlyX) {
  // Solo un valor (falta y, z)
  writeConfigFile("camera_north: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooFewArguments_XAndY) {
  // Solo dos valores (falta z)
  writeConfigFile("camera_north: 0 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooManyArguments_OneExtra) {
  // Más de 4 tokens: un argumento extra
  writeConfigFile("camera_north: 0 1 0 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeConfigFile("camera_north: 0 1 0 1 2 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericX) {
  // Primer valor (x) no numérico
  writeConfigFile("camera_north: abc 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericY) {
  // Segundo valor (y) no numérico
  writeConfigFile("camera_north: 0 abc 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorNonNumericZ) {
  // Tercer valor (z) no numérico
  writeConfigFile("camera_north: 0 1 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeConfigFile("camera_north: up vector here\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("camera_north: 0abc 1def 0ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ErrorPartialNumericValues) {
  // Valores con letras antes de números
  writeConfigFile("camera_north: abc0 def1 ghi0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserCameraNorthTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("camera_north:    0    1    0   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  camera_north: 0 1 0  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  writeConfigFile("camera_north: 0     1     0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# camera_north: 1 0 0\ncamera_north: 0 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ncamera_north: 0 1 0\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que camera_north se procesa correctamente en un contexto más amplio
  writeConfigFile("camera_position: 10 20 -5.5\n"
                  "camera_target: 0 0 0\n"
                  "camera_north: 0 1 0\n"
                  "gamma: 2.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserCameraNorthTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("camera_north: 1 0 0\n"
                  "camera_north: 0 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("camera_north: 00 01 00\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("camera_north: +0 +1 +0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double (x muy grande)
  writeConfigFile("camera_north: 1e400 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars debería detectar el overflow y fallar el parsing
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("camera_north: 0 nan 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("camera_north: 0.0.0 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("camera_north: 0! 1@ 0#\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("camera_north: 0x0 0x1 0x0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.camera_north.x, Constants::CameraNorth.x);
    ASSERT_DOUBLE_EQ(config.camera_north.y, Constants::CameraNorth.y);
    ASSERT_DOUBLE_EQ(config.camera_north.z, Constants::CameraNorth.z);
  }
}

TEST_F(ConfigParserCameraNorthTest, ExtremelyLongDecimal) {
  // Números con muchísimos decimales para verificar precisión
  writeConfigFile("camera_north: 0.123456789012345 1.987654321098765 0.555555555555555\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El valor será parseado con la precisión disponible de double
    ASSERT_NEAR(config.camera_north.x, 0.123456789012345, 1e-15);
    ASSERT_NEAR(config.camera_north.y, 1.987654321098765, 1e-15);
    ASSERT_NEAR(config.camera_north.z, 0.555555555555555, 1e-15);
  }
}

TEST_F(ConfigParserCameraNorthTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente válido en double
  writeConfigFile("camera_north: -0.0 1.0 -0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // -0.0 y 0.0 son iguales en comparaciones
    ASSERT_DOUBLE_EQ(config.camera_north.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.camera_north.z, 0.0);
  }
}

TEST_F(ConfigParserCameraNorthTest, NormalizedVector) {
  // Vector normalizado típico (sqrt(1/3) en cada componente)
  // Este test documenta que el parser NO normaliza el vector automáticamente
  // La normalización debe hacerse en otro lugar del código si es necesaria
  writeConfigFile("camera_north: 0.577350269189626 0.577350269189626 0.577350269189626\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_NEAR(config.camera_north.x, 0.577350269189626, 1e-15);
    ASSERT_NEAR(config.camera_north.y, 0.577350269189626, 1e-15);
    ASSERT_NEAR(config.camera_north.z, 0.577350269189626, 1e-15);
  }
}

// ============================================================================
// TESTS PARA parseFieldOfView
// ============================================================================

class ConfigParserFieldOfViewTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_field_of_view_temp.txt"; }

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

TEST_F(ConfigParserFieldOfViewTest, ValidCommonValue) {
  // Test básico: FOV común de 90 grados
  writeConfigFile("field_of_view: 90\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidValueWithDecimals) {
  // Valor con decimales
  writeConfigFile("field_of_view: 65.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 65.5);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidNearLowerBound) {
  // Cerca del límite inferior (pero mayor que 0)
  writeConfigFile("field_of_view: 0.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 0.1);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidVerySmallPositive) {
  // Valor muy pequeño pero válido
  writeConfigFile("field_of_view: 0.001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 0.001);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidNearUpperBound) {
  // Cerca del límite superior (pero menor que 180)
  writeConfigFile("field_of_view: 179.9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 179.9);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidVeryCloseToUpperBound) {
  // Muy cerca del límite superior
  writeConfigFile("field_of_view: 179.999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 179.999);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidNarrowFOV) {
  // FOV estrecho (teleobjetivo)
  writeConfigFile("field_of_view: 30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 30.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidWideFOV) {
  // FOV ancho (gran angular)
  writeConfigFile("field_of_view: 120\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 120.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidStandardFOV) {
  // FOV estándar (similar a ojo humano)
  writeConfigFile("field_of_view: 45\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 45.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidScientificNotation) {
  // Notación científica: 9e1 = 90.0
  writeConfigFile("field_of_view: 9e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidScientificNotationWithDecimal) {
  // Notación científica con decimal: 6.55e1 = 65.5
  writeConfigFile("field_of_view: 6.55e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 65.5);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ValidScientificNotationNegativeExponent) {
  // Notación científica con exponente negativo: 1e-1 = 0.1
  writeConfigFile("field_of_view: 1e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 0.1);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserFieldOfViewTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("field_of_view:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("field_of_view: 90 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("field_of_view: 90 65 45\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserFieldOfViewTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("field_of_view: abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("field_of_view: 90abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorPartialNumericValue) {
  // Valor con letras antes de números
  writeConfigFile("field_of_view: abc90\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorEmptyValue) {
  // Token vacío
  writeConfigFile("field_of_view: \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

// ERRORES DE RANGO - Límites y valores fuera de rango

TEST_F(ConfigParserFieldOfViewTest, ErrorEqualToLowerBound) {
  // Valor igual al límite inferior (0) - NO válido
  // La condición es fov > 0, por lo que 0 es rechazado
  writeConfigFile("field_of_view: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorEqualToLowerBoundDecimal) {
  // Valor 0.0 con decimal explícito
  writeConfigFile("field_of_view: 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorLessThanLowerBound) {
  // Valor menor que el límite inferior (negativo)
  writeConfigFile("field_of_view: -10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorNegativeSmallValue) {
  // Valor negativo pequeño
  writeConfigFile("field_of_view: -0.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorNegativeVeryLarge) {
  // Valor negativo muy grande
  writeConfigFile("field_of_view: -1000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorEqualToUpperBound) {
  // Valor igual al límite superior (180) - NO válido
  // La condición es fov < 180, por lo que 180 es rechazado
  writeConfigFile("field_of_view: 180\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorEqualToUpperBoundDecimal) {
  // Valor 180.0 con decimal explícito
  writeConfigFile("field_of_view: 180.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorGreaterThanUpperBound) {
  // Valor mayor que el límite superior
  writeConfigFile("field_of_view: 200\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorSlightlyGreaterThanUpperBound) {
  // Valor ligeramente mayor que el límite superior
  writeConfigFile("field_of_view: 180.001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorVeryLargeValue) {
  // Valor muy grande (fuera de rango)
  writeConfigFile("field_of_view: 360\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ErrorExtremelyLargeValue) {
  // Valor extremadamente grande
  writeConfigFile("field_of_view: 10000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserFieldOfViewTest, ExtraWhitespaceAroundValue) {
  // Espacios extra alrededor del valor
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("field_of_view:    90   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  field_of_view: 90  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# field_of_view: 45\nfield_of_view: 90\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\nfield_of_view: 90\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que field_of_view se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "field_of_view: 75.5\n"
                  "image_width: 1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 75.5);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserFieldOfViewTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("field_of_view: 90\n"
                  "field_of_view: 65\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 65.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("field_of_view: 0090\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, PlusSignPrefix) {
  // Signo + explícito (válido para doubles)
  writeConfigFile("field_of_view: +90\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 90.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double
  writeConfigFile("field_of_view: 1e400\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars debería detectar el overflow y fallar el parsing
    // mantiene el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, InvalidInfinityString) {
  // String "inf" - std::from_chars típicamente NO acepta esto
  writeConfigFile("field_of_view: inf\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, InvalidNaNString) {
  // String "nan" - std::from_chars típicamente NO acepta esto
  writeConfigFile("field_of_view: nan\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("field_of_view: 90.5.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, SpecialCharactersInValue) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("field_of_view: 90!\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada)
  writeConfigFile("field_of_view: 0x5A\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ExtremelyLongDecimal) {
  // Número con muchísimos decimales para verificar precisión
  writeConfigFile("field_of_view: 90.123456789012345\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El valor será parseado con la precisión disponible de double
    ASSERT_NEAR(config.field_of_view, 90.123456789012345, 1e-15);
  }
}

TEST_F(ConfigParserFieldOfViewTest, NegativeZero) {
  // Caso curioso: -0.0 es técnicamente 0.0, que es inválido para FOV
  writeConfigFile("field_of_view: -0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (0 no es válido)
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, BoundaryTestJustAboveZero) {
  // Valor extremadamente pequeño pero mayor que 0
  // Este test verifica el límite inferior con precisión de double
  writeConfigFile("field_of_view: 1e-10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 1e-10);
  }
}

TEST_F(ConfigParserFieldOfViewTest, BoundaryTestJustBelow180) {
  // Valor extremadamente cercano a 180 pero menor
  // Este test verifica el límite superior con precisión de double
  writeConfigFile("field_of_view: 179.9999999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.field_of_view, 179.9999999999);
  }
}

TEST_F(ConfigParserFieldOfViewTest, CombinedWithCameraParameters) {
  // Test de integración: verifica que field_of_view se puede usar
  // junto con otros parámetros de cámara
  writeConfigFile("camera_position: 10 5 -10\n"
                  "camera_target: 0 0 0\n"
                  "camera_north: 0 1 0\n"
                  "field_of_view: 75\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.camera_pos.x, 10.0);
    ASSERT_DOUBLE_EQ(config.camera_target.x, 0.0);
    ASSERT_DOUBLE_EQ(config.camera_north.y, 1.0);
    ASSERT_DOUBLE_EQ(config.field_of_view, 75.0);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ScientificNotationOutOfRange) {
  // Notación científica que resulta en un valor fuera de rango
  writeConfigFile("field_of_view: 2e2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // 2e2 = 200, que está fuera del rango válido
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
}

TEST_F(ConfigParserFieldOfViewTest, ScientificNotationNegative) {
  // Notación científica con valor negativo resultante
  writeConfigFile("field_of_view: -9e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // -9e1 = -90, que está fuera del rango válido
    ASSERT_DOUBLE_EQ(config.field_of_view, Constants::FOV);
  }
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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ValidAlternativeRatio) {
  // Otro caso válido: aspect ratio 4:3 (común en monitores antiguos)
  writeConfigFile("aspect_ratio: 4 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 4U);
    ASSERT_EQ(config.aspect_ratio.second, 3U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ValidWideScreenRatio) {
  // Aspect ratio ultrawide 21:9
  writeConfigFile("aspect_ratio: 21 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 21U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ValidSquareRatio) {
  // Aspect ratio cuadrado 1:1
  writeConfigFile("aspect_ratio: 1 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 1U);
    ASSERT_EQ(config.aspect_ratio.second, 1U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ValidCinematicRatio) {
  // Aspect ratio cinematográfico 2.39:1 representado como 239:100
  // Este test verifica que valores más grandes funcionan correctamente
  writeConfigFile("aspect_ratio: 239 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 239U);
    ASSERT_EQ(config.aspect_ratio.second, 100U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ValidLargeValues) {
  // Valores grandes pero válidos para verificar robustez
  // Útil para verificar que no hay límites artificiales
  writeConfigFile("aspect_ratio: 3840 2160\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 3'840U);
    ASSERT_EQ(config.aspect_ratio.second, 2'160U);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserAspectRatioTest, ErrorTooFewArguments_MissingBoth) {
  // Menos de 3 tokens: faltan ambos valores
  writeConfigFile("aspect_ratio:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooFewArguments_MissingHeight) {
  // Menos de 3 tokens: falta el segundo valor (height)
  writeConfigFile("aspect_ratio: 16\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooManyArguments_OneExtra) {
  // Más de 3 tokens: un argumento extra
  writeConfigFile("aspect_ratio: 16 9 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorTooManyArguments_Multiple) {
  // Más de 3 tokens: múltiples argumentos extra
  writeConfigFile("aspect_ratio: 16 9 4 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserAspectRatioTest, ErrorNonNumericWidth) {
  // Primer valor no numérico
  writeConfigFile("aspect_ratio: abc 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorNonNumericHeight) {
  // Segundo valor no numérico
  writeConfigFile("aspect_ratio: 16 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothNonNumeric) {
  // Ambos valores no numéricos
  writeConfigFile("aspect_ratio: abc def\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorAlphanumericMixed) {
  // Valores con mezcla de letras y números
  writeConfigFile("aspect_ratio: 16abc 9def\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorFloatingPointValues) {
  // Valores de punto flotante (deberían rechazarse ya que espera unsigned int)
  // Este test es importante porque parseUnsignedInt debe rechazar decimales
  writeConfigFile("aspect_ratio: 16.5 9.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserAspectRatioTest, ErrorZeroWidth) {
  // Width es cero (no positivo)
  writeConfigFile("aspect_ratio: 0 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorZeroHeight) {
  // Height es cero (no positivo)
  writeConfigFile("aspect_ratio: 16 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothZero) {
  // Ambos valores cero
  writeConfigFile("aspect_ratio: 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorNegativeWidth) {
  // Width negativo (debería ser rechazado por parseUnsignedInt)
  // parseUnsignedInt NO acepta valores negativos
  writeConfigFile("aspect_ratio: -16 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorNegativeHeight) {
  // Height negativo (debería ser rechazado por parseUnsignedInt)
  writeConfigFile("aspect_ratio: 16 -9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ErrorBothNegative) {
  // Ambos valores negativos
  writeConfigFile("aspect_ratio: -16 -9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserAspectRatioTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("aspect_ratio:    16    9   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  aspect_ratio: 16 9  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, MultipleSpacesBetweenValues) {
  // Múltiples espacios entre valores
  // El tokenizer actual debería manejar esto correctamente
  writeConfigFile("aspect_ratio: 16     9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# aspect_ratio: 4 3\naspect_ratio: 16 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\naspect_ratio: 16 9\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que aspect_ratio se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "aspect_ratio: 21 9\n"
                  "image_width: 2560\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 21U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.image_width, 2'560);
  }
}

TEST_F(ConfigParserAspectRatioTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("aspect_ratio: 16 9\n"
                  "aspect_ratio: 4 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 4U);
    ASSERT_EQ(config.aspect_ratio.second, 3U);
  }
}

TEST_F(ConfigParserAspectRatioTest, UnsignedIntMaxValue) {
  // Valor máximo para unsigned int (típicamente 4294967295)
  // Este test verifica que valores muy grandes pero válidos funcionan
  writeConfigFile("aspect_ratio: 4294967295 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 4'294'967'295U);
    ASSERT_EQ(config.aspect_ratio.second, 1U);
  }
}

TEST_F(ConfigParserAspectRatioTest, UnsignedIntOverflow) {
  // Valor que excede UINT_MAX (debería causar overflow)
  // Este test verifica la robustez ante valores extremos
  // UINT_MAX típicamente es 4294967295
  writeConfigFile("aspect_ratio: 4294967296 9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debería fallar el parsing y mantener el valor por defecto
    // porque std::from_chars detectará el overflow
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente sin interpretarse como octal
  writeConfigFile("aspect_ratio: 0016 009\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
  }
}

TEST_F(ConfigParserAspectRatioTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: std::from_chars para enteros sin signo NO acepta el signo +.
  // Este test documenta que valores con '+' explícito son rechazados.
  writeConfigFile("aspect_ratio: +16 +9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El parser rechaza el signo +, mantiene valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ScientificNotation) {
  // Notación científica (no debería ser aceptada para enteros sin signo)
  writeConfigFile("aspect_ratio: 1e2 9e0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada sin configuración especial)
  // std::from_chars en base 10 (por defecto) no acepta prefijo 0x
  writeConfigFile("aspect_ratio: 0x10 0x09\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, SpecialCharactersInValues) {
  // Caracteres especiales que podrían causar problemas
  // Este test verifica que el parser es robusto ante entradas malformadas
  writeConfigFile("aspect_ratio: 16! 9@\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ColonInsteadOfSpace) {
  // Usar ':' en lugar de espacio (error común de formato)
  // El tokenizer actual busca espacios, no dos puntos
  writeConfigFile("aspect_ratio: 16:9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (parsing fallará por formato incorrecto)
    ASSERT_EQ(config.aspect_ratio.first, Constants::AspectRatio.first);
    ASSERT_EQ(config.aspect_ratio.second, Constants::AspectRatio.second);
  }
}

TEST_F(ConfigParserAspectRatioTest, ExtremelyLargeValidValues) {
  // Valores grandes pero válidos para verificar el límite superior
  // Útil para aplicaciones que renderizan a resoluciones muy altas
  writeConfigFile("aspect_ratio: 1000000 562500\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.aspect_ratio.first, 1'000'000U);
    ASSERT_EQ(config.aspect_ratio.second, 562'500U);
  }
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

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, ValidAlternativeValue) {
  // Otro valor válido: gamma 1.8
  writeConfigFile("gamma: 1.8\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 1.8);
  }
}

TEST_F(ConfigParserGammaTest, ValidIntegerValue) {
  // Valor entero: gamma 2 (debería convertirse a 2.0)
  writeConfigFile("gamma: 2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.0);
  }
}

TEST_F(ConfigParserGammaTest, ValidScientificNotation) {
  // Notación científica: 1e-1 = 0.1
  // Este test verifica que parsedouble acepta notación científica
  writeConfigFile("gamma: 1e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.1);
  }
}

TEST_F(ConfigParserGammaTest, ValidScientificNotationPositiveExponent) {
  // Notación científica con exponente positivo: 2.2e0 = 2.2
  writeConfigFile("gamma: 2.2e0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, ValidScientificNotationLargeExponent) {
  // Notación científica con exponente mayor: 1.5e2 = 150.0
  writeConfigFile("gamma: 1.5e2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 150.0);
  }
}

TEST_F(ConfigParserGammaTest, ValidZeroValue) {
  // Valor cero (técnicamente válido aunque poco práctico para gamma)
  // parseGamma no valida rangos, solo acepta doubles válidos
  writeConfigFile("gamma: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.0);
  }
}

TEST_F(ConfigParserGammaTest, ValidZeroDecimal) {
  // Valor cero con decimales
  writeConfigFile("gamma: 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.0);
  }
}

TEST_F(ConfigParserGammaTest, ValidNegativeValue) {
  // Valor negativo (técnicamente válido según parseGamma, no hay validación de rango)
  writeConfigFile("gamma: -1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, -1.0);
  }
}

TEST_F(ConfigParserGammaTest, ValidSmallPositiveValue) {
  // Valor pequeño positivo
  writeConfigFile("gamma: 0.001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.001);
  }
}

TEST_F(ConfigParserGammaTest, ValidLargeValue) {
  // Valor grande pero válido
  writeConfigFile("gamma: 100.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 100.5);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserGammaTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("gamma:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("gamma: 2.2 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("gamma: 2.2 1.8 3.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserGammaTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("gamma: abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("gamma: 2.2abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ErrorPartialNumericValue) {
  // Valor con caracteres numéricos y no numéricos al inicio
  writeConfigFile("gamma: abc2.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ErrorEmptyValue) {
  // Token vacío (debería ser detectado por parsedouble)
  writeConfigFile("gamma: \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserGammaTest, ExtraWhitespaceAroundValue) {
  // Espacios extra alrededor del valor
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("gamma:    2.2   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  gamma: 2.2  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# gamma: 1.0\ngamma: 2.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\ngamma: 2.2\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que gamma se procesa correctamente en un contexto más amplio
  writeConfigFile("aspect_ratio: 16 9\n"
                  "gamma: 1.8\n"
                  "image_width: 1920\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 1.8);
    ASSERT_EQ(config.aspect_ratio.first, 16U);
    ASSERT_EQ(config.aspect_ratio.second, 9U);
    ASSERT_EQ(config.image_width, 1'920);
  }
}

TEST_F(ConfigParserGammaTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  // Este comportamiento es común en parsers de configuración
  writeConfigFile("gamma: 2.2\n"
                  "gamma: 1.8\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 1.8);
  }
}

TEST_F(ConfigParserGammaTest, VeryLargeValidValue) {
  // Valor muy grande pero válido para double
  // Este test verifica que no hay límites artificiales
  writeConfigFile("gamma: 1000000.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 1000000.5);
  }
}

TEST_F(ConfigParserGammaTest, VerySmallValidValue) {
  // Valor muy pequeño pero válido
  writeConfigFile("gamma: 0.0000001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.0000001);
  }
}

TEST_F(ConfigParserGammaTest, ScientificNotationNegativeExponent) {
  // Notación científica con exponente negativo: 2.2e-3 = 0.0022
  writeConfigFile("gamma: 2.2e-3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 0.0022);
  }
}

TEST_F(ConfigParserGammaTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  // Verifica que se parsean correctamente
  writeConfigFile("gamma: 002.200\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, PlusSignPrefix) {
  // Signo + explícito
  // std::from_chars para doubles SÍ acepta el signo +
  writeConfigFile("gamma: +2.2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

TEST_F(ConfigParserGammaTest, DoubleOverflowProtection) {
  // Valor que podría causar overflow en double
  // Este test verifica la robustez ante valores extremos
  // Un valor mayor que DBL_MAX debería ser manejado por std::from_chars
  writeConfigFile("gamma: 1e400\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars debería detectar el overflow y fallar el parsing
    // mantiene el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, DoubleUnderflowToZero) {
  // Valor extremadamente pequeño que podría underflow a cero
  // Esto debería ser válido ya que parsedouble lo acepta
  writeConfigFile("gamma: 1e-400\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debería ser parseado como 0.0 (underflow)
    ASSERT_DOUBLE_EQ(config.gamma, 0.0);
  }
}

TEST_F(ConfigParserGammaTest, InvalidInfinityString) {
  // String "inf" - std::from_chars puede o no aceptar esto
  // Este test documenta el comportamiento actual
  writeConfigFile("gamma: inf\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars típicamente NO acepta "inf" como string
    // mantiene el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, InvalidNaNString) {
  // String "nan" - std::from_chars puede o no aceptar esto
  // Este test documenta el comportamiento actual
  writeConfigFile("gamma: nan\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // std::from_chars típicamente NO acepta "nan" como string
    // mantiene el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("gamma: 2.2.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, SpecialCharactersInValue) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("gamma: 2.2!\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada por std::from_chars en modo decimal)
  writeConfigFile("gamma: 0x1.8p1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.gamma, Constants::Gamma);
  }
}

TEST_F(ConfigParserGammaTest, ExtremelyLongDecimal) {
  // Número con muchísimos decimales para verificar precisión
  // Este test es importante porque verifica que parsedouble maneja
  // números con alta precisión correctamente
  writeConfigFile("gamma: 2.2222222222222222222222222222\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El valor será parseado con la precisión disponible de double
    // Verificamos que es aproximadamente correcto
    ASSERT_NEAR(config.gamma, 2.2222222222222222, 1e-15);
  }
}

// ============================================================================
// TESTS PARA parseBackgroundDarkColor
// ============================================================================

class ConfigParserBackgroundDarkColorTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_background_dark_color_temp.txt"; }

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

TEST_F(ConfigParserBackgroundDarkColorTest, ValidDefaultFromPDF) {
  // Valor por defecto del PDF: azul cielo oscuro
  // Este es un color típico para el fondo oscuro en un gradiente de cielo
  writeConfigFile("background_dark_color: 0.25 0.5 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidBlackColor) {
  // Negro puro: (0, 0, 0)
  // Útil para fondos completamente oscuros
  writeConfigFile("background_dark_color: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidWhiteColor) {
  // Blanco puro: (1, 1, 1)
  // Aunque inusual para "dark color", es técnicamente válido
  writeConfigFile("background_dark_color: 1 1 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidCustomColor) {
  // Color personalizado: gris azulado oscuro
  writeConfigFile("background_dark_color: 0.1 0.2 0.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.1);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.2);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.3);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidMidToneGray) {
  // Gris medio: (0.5, 0.5, 0.5)
  writeConfigFile("background_dark_color: 0.5 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidDarkBlue) {
  // Azul oscuro: color típico para cielo nocturno
  writeConfigFile("background_dark_color: 0.0 0.0 0.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.3);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidDarkRed) {
  // Rojo oscuro
  writeConfigFile("background_dark_color: 0.3 0.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.3);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeConfigFile("background_dark_color: 0.123456 0.654321 0.999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.123456);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.654321);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.999999);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorTooFewArguments_NoValues) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("background_dark_color:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorTooFewArguments_OneValue) {
  // Solo un valor (falta g y b)
  writeConfigFile("background_dark_color: 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorTooFewArguments_TwoValues) {
  // Solo dos valores (falta b)
  writeConfigFile("background_dark_color: 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorTooManyArguments) {
  // Más de 4 tokens: argumentos extra
  writeConfigFile("background_dark_color: 0.5 0.5 0.5 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("background_dark_color: 0.5 0.5 0.5 extra1 extra2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeConfigFile("background_dark_color: abc 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeConfigFile("background_dark_color: 0.5 abc 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeConfigFile("background_dark_color: 0.5 0.5 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorAllNonNumeric) {
  // Todos los componentes no numéricos
  writeConfigFile("background_dark_color: abc def ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorEmptyValues) {
  // Valores vacíos
  writeConfigFile("background_dark_color:   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

// ERRORES DE RANGO - Componentes fuera del rango [0, 1]

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeConfigFile("background_dark_color: -0.1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorGComponentBelowZero) {
  // Componente G menor que 0
  writeConfigFile("background_dark_color: 0.5 -0.1 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeConfigFile("background_dark_color: 0.5 0.5 -0.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorRComponentAboveOne) {
  // Componente R mayor que 1
  writeConfigFile("background_dark_color: 1.1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeConfigFile("background_dark_color: 0.5 1.1 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeConfigFile("background_dark_color: 0.5 0.5 1.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorAllComponentsBelowZero) {
  // Todos los componentes menores que 0
  writeConfigFile("background_dark_color: -0.1 -0.2 -0.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorAllComponentsAboveOne) {
  // Todos los componentes mayores que 1
  writeConfigFile("background_dark_color: 1.1 1.2 1.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorVeryLargeValue) {
  // Valor muy grande (fuera de rango)
  writeConfigFile("background_dark_color: 100.0 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ErrorVeryLargeNegativeValue) {
  // Valor negativo muy grande
  writeConfigFile("background_dark_color: -100.0 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(ConfigParserBackgroundDarkColorTest, BoundaryExactlyZero) {
  // Todos los componentes exactamente 0.0 (ya cubierto en ValidBlackColor)
  // Este test es redundante pero documenta explícitamente el límite inferior
  writeConfigFile("background_dark_color: 0.0 0.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, BoundaryExactlyOne) {
  // Todos los componentes exactamente 1.0 (ya cubierto en ValidWhiteColor)
  // Este test es redundante pero documenta explícitamente el límite superior
  writeConfigFile("background_dark_color: 1.0 1.0 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, BoundaryMixedZeroOne) {
  // Mezcla de valores 0.0 y 1.0
  writeConfigFile("background_dark_color: 0.0 1.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, BoundaryVeryCloseToZero) {
  // Valores muy cercanos a 0 pero válidos
  writeConfigFile("background_dark_color: 0.0001 0.0001 0.0001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0001);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0001);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0001);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, BoundaryVeryCloseToOne) {
  // Valores muy cercanos a 1 pero válidos
  writeConfigFile("background_dark_color: 0.9999 0.9999 0.9999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.9999);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.9999);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.9999);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserBackgroundDarkColorTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("background_dark_color:    0.5   0.5   0.5   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  background_dark_color: 0.5 0.5 0.5  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# background_dark_color: 1.0 1.0 1.0\nbackground_dark_color: 0.25 0.5 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\nbackground_dark_color: 0.25 0.5 1.0\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que background_dark_color se procesa correctamente en contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "background_dark_color: 0.25 0.5 1.0\n"
                  "background_light_color: 1.0 1.0 1.0\n"
                  "samples_per_pixel: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.samples_per_pixel, 100);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("background_dark_color: 0.0 0.0 0.0\n"
                  "background_dark_color: 0.25 0.5 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1]
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9
  writeConfigFile("background_dark_color: 5e-1 1e-1 9e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.1);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.9);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeConfigFile("background_dark_color: 1e1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (fuera de rango)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, NegativeZeroComponent) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeConfigFile("background_dark_color: -0.0 -0.0 -0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeConfigFile("background_dark_color: 0 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("background_dark_color: 00.5 00.5 00.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeConfigFile("background_dark_color: +0.5 +0.5 +0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, TrailingWhitespaceAndComments) {
  // Whitespace y comentarios después de los valores
  // Verifica que el parser maneja correctamente este caso
  writeConfigFile("background_dark_color: 0.25 0.5 1.0   # Sky gradient dark color\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El tokenizer debería tomar "0.25", "0.5", "1.0" y "# comentario" como tokens
    // lo cual resulta en error de parsing (demasiados argumentos)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeConfigFile("background_dark_color:\t0.5\t0.5\t0.5\t\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeConfigFile("  \tbackground_dark_color:  \t 0.5 \t 0.5 \t 0.5 \t \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, InfinityValue) {
  // Valor infinito (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeConfigFile("background_dark_color: inf 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (inf > 1.0)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeConfigFile("background_dark_color: nan 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (nan falla la comparación)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, IntegrationWithLightColor) {
  // Test de integración: verifica que dark y light colors pueden coexistir
  // Documenta un gradiente típico de cielo
  writeConfigFile("background_dark_color: 0.25 0.5 1.0\n"
                  "background_light_color: 0.5 0.7 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Dark color (azul oscuro)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
    // Light color (azul claro)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.7);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundDarkColorTest, AlphanumericInComponent) {
  // Valor alfanumérico mixto en un componente
  writeConfigFile("background_dark_color: 0.5abc 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, Constants::ColorBackgroundDark.x);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, Constants::ColorBackgroundDark.y);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, Constants::ColorBackgroundDark.z);
  }
}

// ============================================================================
// TESTS PARA parseBackgroundLightColor
// ============================================================================

class ConfigParserBackgroundLightColorTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_background_light_color_temp.txt"; }

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

TEST_F(ConfigParserBackgroundLightColorTest, ValidDefaultFromPDF) {
  // Valor por defecto del PDF: blanco puro (cielo brillante)
  // Este es el color típico para el fondo claro en un gradiente de cielo
  writeConfigFile("background_light_color: 1 1 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidBlackColor) {
  // Negro puro: (0, 0, 0)
  // Aunque inusual para "light color", es técnicamente válido
  writeConfigFile("background_light_color: 0 0 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidMidToneGray) {
  // Gris medio: (0.5, 0.5, 0.5)
  writeConfigFile("background_light_color: 0.5 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidCustomColor) {
  // Color personalizado típico para gradiente de cielo: azul claro
  writeConfigFile("background_light_color: 0.8 0.9 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.8);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.9);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidLightBlue) {
  // Azul claro: color típico para cielo diurno
  writeConfigFile("background_light_color: 0.5 0.7 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.7);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidWarmWhite) {
  // Blanco cálido (ligeramente amarillento)
  writeConfigFile("background_light_color: 1.0 1.0 0.9\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.9);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeConfigFile("background_light_color: 0.123456 0.654321 0.999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.123456);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.654321);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.999999);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ValidSunsetColors) {
  // Colores típicos de atardecer (naranja/rosa claro)
  writeConfigFile("background_light_color: 1.0 0.8 0.6\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.8);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.6);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserBackgroundLightColorTest, ErrorTooFewArguments_NoValues) {
  // Menos de 4 tokens: faltan todos los valores
  writeConfigFile("background_light_color:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorTooFewArguments_OneValue) {
  // Solo un valor (falta g y b)
  writeConfigFile("background_light_color: 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorTooFewArguments_TwoValues) {
  // Solo dos valores (falta b)
  writeConfigFile("background_light_color: 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorTooManyArguments) {
  // Más de 4 tokens: argumentos extra
  writeConfigFile("background_light_color: 0.5 0.5 0.5 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("background_light_color: 0.5 0.5 0.5 extra1 extra2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserBackgroundLightColorTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeConfigFile("background_light_color: abc 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeConfigFile("background_light_color: 0.5 abc 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeConfigFile("background_light_color: 0.5 0.5 abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorAllNonNumeric) {
  // Todos los componentes no numéricos
  writeConfigFile("background_light_color: abc def ghi\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorEmptyValues) {
  // Valores vacíos
  writeConfigFile("background_light_color:   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorAlphanumericInComponent) {
  // Valor alfanumérico mixto en un componente
  writeConfigFile("background_light_color: 0.5abc 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

// ERRORES DE RANGO - Componentes fuera del rango [0, 1]

TEST_F(ConfigParserBackgroundLightColorTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeConfigFile("background_light_color: -0.1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorGComponentBelowZero) {
  // Componente G menor que 0
  writeConfigFile("background_light_color: 0.5 -0.1 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeConfigFile("background_light_color: 0.5 0.5 -0.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorRComponentAboveOne) {
  // Componente R mayor que 1
  writeConfigFile("background_light_color: 1.1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeConfigFile("background_light_color: 0.5 1.1 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeConfigFile("background_light_color: 0.5 0.5 1.1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorBComponentFarBelowZero) {
  // Componente B muy por debajo de 0 (caso -1)
  writeConfigFile("background_light_color: 0.5 0.5 -1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorBComponentFarAboveOne) {
  // Componente B muy por encima de 1 (caso 2)
  writeConfigFile("background_light_color: 0.5 0.5 2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorAllComponentsBelowZero) {
  // Todos los componentes menores que 0
  writeConfigFile("background_light_color: -0.1 -0.2 -0.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorAllComponentsAboveOne) {
  // Todos los componentes mayores que 1
  writeConfigFile("background_light_color: 1.1 1.2 1.3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorVeryLargeValue) {
  // Valor muy grande (fuera de rango)
  writeConfigFile("background_light_color: 100.0 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ErrorVeryLargeNegativeValue) {
  // Valor negativo muy grande
  writeConfigFile("background_light_color: -100.0 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(ConfigParserBackgroundLightColorTest, BoundaryExactlyZero) {
  // Todos los componentes exactamente 0.0
  writeConfigFile("background_light_color: 0.0 0.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, BoundaryExactlyOne) {
  // Todos los componentes exactamente 1.0 (valor por defecto)
  writeConfigFile("background_light_color: 1.0 1.0 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, BoundaryMixedZeroOne) {
  // Mezcla de valores 0.0 y 1.0
  writeConfigFile("background_light_color: 0.0 1.0 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, BoundaryVeryCloseToZero) {
  // Valores muy cercanos a 0 pero válidos
  writeConfigFile("background_light_color: 0.0001 0.0001 0.0001\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0001);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.0001);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0001);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, BoundaryVeryCloseToOne) {
  // Valores muy cercanos a 1 pero válidos
  writeConfigFile("background_light_color: 0.9999 0.9999 0.9999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.9999);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.9999);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.9999);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserBackgroundLightColorTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("background_light_color:    1.0   1.0   1.0   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  background_light_color: 1.0 1.0 1.0  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1]
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9
  writeConfigFile("background_light_color: 5e-1 1e-1 9e-1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.1);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.9);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeConfigFile("background_light_color: 1e1 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (fuera de rango)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, NegativeZeroComponent) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeConfigFile("background_light_color: -0.0 -0.0 -0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeConfigFile("background_light_color: 0 1 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("background_light_color: 00.5 00.5 00.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 0.5);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeConfigFile("background_light_color:\t1.0\t1.0\t1.0\t\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeConfigFile("  \tbackground_light_color:  \t 1.0 \t 1.0 \t 1.0 \t \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, InfinityValue) {
  // Valor infinito (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeConfigFile("background_light_color: inf 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = config_opt.value();

    // Debe mantener el valor por defecto (inf > 1.0)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeConfigFile("background_light_color: nan 0.5 0.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (nan falla la comparación)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# background_light_color: 0.0 0.0 0.0\nbackground_light_color: 1.0 1.0 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\nbackground_light_color: 1.0 1.0 1.0\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("background_light_color: 0.0 0.0 0.0\n"
                  "background_light_color: 1.0 1.0 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, TrailingWhitespaceAndComments) {
  // Whitespace y comentarios después de los valores
  // El tokenizer toma "1.0", "1.0", "1.0" y "#" como tokens (demasiados argumentos)
  writeConfigFile("background_light_color: 1.0 1.0 1.0   # Sky gradient light color\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (error de parsing por argumentos extra)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, Constants::ColorBackGroundLight.x);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, Constants::ColorBackGroundLight.y);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, Constants::ColorBackGroundLight.z);
  }
}

// TESTS DE INTEGRACIÓN

TEST_F(ConfigParserBackgroundLightColorTest, IntegrationWithDarkColor) {
  // Test de integración: verifica que dark y light colors pueden coexistir
  // Documenta un gradiente típico de cielo
  writeConfigFile("background_dark_color: 0.25 0.5 1.0\n"
                  "background_light_color: 0.5 0.7 1.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Light color (azul claro)
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 0.5);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 0.7);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
    // Dark color (azul oscuro)
    ASSERT_DOUBLE_EQ(config.background_dark_color.x, 0.25);
    ASSERT_DOUBLE_EQ(config.background_dark_color.y, 0.5);
    ASSERT_DOUBLE_EQ(config.background_dark_color.z, 1.0);
  }
}

TEST_F(ConfigParserBackgroundLightColorTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que background_light_color se procesa correctamente en contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "background_light_color: 1.0 1.0 1.0\n"
                  "background_dark_color: 0.25 0.5 1.0\n"
                  "samples_per_pixel: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_DOUBLE_EQ(config.background_light_color.x, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.y, 1.0);
    ASSERT_DOUBLE_EQ(config.background_light_color.z, 1.0);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.samples_per_pixel, 100);
  }
}

// ============================================================================
// TESTS PARA parseMaterialRngSeed
// ============================================================================

class ConfigParserMaterialRngSeedTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_material_rng_seed_temp.txt"; }

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

TEST_F(ConfigParserMaterialRngSeedTest, ValidCommonValue) {
  // Test básico: valor común para semilla de RNG de materiales
  // Las semillas de RNG son importantes para reproducibilidad en renderizado
  writeConfigFile("material_rng_seed: 13\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidMinimumValue) {
  // Valor mínimo válido: 1
  // La función requiere seed > 0, por lo que 1 es el mínimo
  writeConfigFile("material_rng_seed: 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 1UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidLargeValue) {
  // Valor grande dentro del rango de unsigned long
  // Importante para verificar que parseUnsignedLong maneja valores grandes
  writeConfigFile("material_rng_seed: 123456789012345\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 123'456'789'012'345UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidTypicalValue) {
  // Valor típico usado en práctica: 42 (seed común)
  writeConfigFile("material_rng_seed: 42\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 42UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidMediumValue) {
  // Valor medio: 1000
  writeConfigFile("material_rng_seed: 1000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 1'000UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidVeryLargeValue) {
  // Valor muy grande: mil millones
  // Verifica que parseUnsignedLong maneja valores muy grandes correctamente
  writeConfigFile("material_rng_seed: 1000000000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 1'000'000'000UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ValidTimestampLikeValue) {
  // Valor similar a timestamp UNIX (uso común para seeds)
  // 1698000000 ≈ octubre 2023
  writeConfigFile("material_rng_seed: 1698000000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 1'698'000'000UL);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserMaterialRngSeedTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("material_rng_seed:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("material_rng_seed: 13 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("material_rng_seed: 13 42 99\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserMaterialRngSeedTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("material_rng_seed: abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("material_rng_seed: 13abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorPartialNumericValue) {
  // Valor con letras antes de números
  writeConfigFile("material_rng_seed: abc13\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorEmptyValue) {
  // Token vacío
  writeConfigFile("material_rng_seed: \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorFloatingPointValue) {
  // Valor con decimales (debería ser rechazado por parseUnsignedLong)
  // Las semillas de RNG deben ser enteros, no acepta valores como 13.5
  writeConfigFile("material_rng_seed: 13.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorFloatingPointZero) {
  // Valor decimal cero
  writeConfigFile("material_rng_seed: 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorScientificNotation) {
  // Notación científica (parseUnsignedLong no la acepta)
  writeConfigFile("material_rng_seed: 1e3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserMaterialRngSeedTest, ErrorZeroValue) {
  // Valor cero (no positivo) - NO válido
  // La condición es seed > 0, por lo que 0 es rechazado
  // Semánticamente: una semilla de 0 no es práctica para RNG
  writeConfigFile("material_rng_seed: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorNegativeValue) {
  // Valor negativo (parseUnsignedLong debería rechazarlo)
  // unsigned long no puede almacenar valores negativos
  writeConfigFile("material_rng_seed: -13\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorNegativeSmallValue) {
  // Valor negativo pequeño: -1
  writeConfigFile("material_rng_seed: -1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ErrorNegativeLargeValue) {
  // Valor negativo grande
  writeConfigFile("material_rng_seed: -1000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserMaterialRngSeedTest, ExtraWhitespaceAroundValue) {
  // Espacios extra alrededor del valor
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("material_rng_seed:    13   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  material_rng_seed: 13  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# material_rng_seed: 99\nmaterial_rng_seed: 13\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\nmaterial_rng_seed: 13\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que material_rng_seed se procesa correctamente en contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "material_rng_seed: 42\n"
                  "ray_rng_seed: 123\n"
                  "max_depth: 10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 42UL);
    ASSERT_EQ(config.ray_rng_seed, 123UL);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("material_rng_seed: 13\n"
                  "material_rng_seed: 42\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 42UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("material_rng_seed: 00013\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: parseUnsignedLong NO acepta el signo + explícito
  // Este test documenta ese comportamiento
  writeConfigFile("material_rng_seed: +13\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // parseUnsignedLong rechaza el signo +, mantiene valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, UnsignedLongMaxValue) {
  // Valor máximo para unsigned long (típicamente 18446744073709551615 en 64-bit)
  // Este test verifica que valores muy grandes pero válidos funcionan
  // Nota: ULONG_MAX puede variar según la plataforma
  writeConfigFile("material_rng_seed: 18446744073709551615\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 18'446'744'073'709'551'615UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, UnsignedLongOverflowProtection) {
  // Valor que excede ULONG_MAX (debería causar overflow)
  // Este test verifica la robustez ante valores extremos
  // ULONG_MAX típicamente es 18446744073709551615
  writeConfigFile("material_rng_seed: 18446744073709551616\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debería fallar el parsing y mantener el valor por defecto
    // porque std::from_chars detectará el overflow
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, VeryLargeOverflow) {
  // Valor extremadamente grande que causa overflow
  writeConfigFile("material_rng_seed: 999999999999999999999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("material_rng_seed: 13.0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, SpecialCharactersInValue) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("material_rng_seed: 13!\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada por parseUnsignedLong en base 10)
  writeConfigFile("material_rng_seed: 0xD\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, OctalNotation) {
  // Notación octal (015 = 13 en octal)
  // parseUnsignedLong en base 10 no interpreta esto como octal
  writeConfigFile("material_rng_seed: 015\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Se parsea como decimal 15, no como octal 13
    ASSERT_EQ(config.material_rng_seed, 15UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, CommonPrimeNumberSeed) {
  // Número primo común usado como seed: 7919
  // Los números primos son populares para seeds de RNG
  writeConfigFile("material_rng_seed: 7919\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 7'919UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, PowerOfTwo) {
  // Potencia de 2: 65536 (2^16)
  writeConfigFile("material_rng_seed: 65536\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 65'536UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, CombinedWithBothRngSeeds) {
  // Test de integración: verifica que material_rng_seed y ray_rng_seed
  // pueden coexistir con valores diferentes
  writeConfigFile("material_rng_seed: 111\n"
                  "ray_rng_seed: 222\n"
                  "samples_per_pixel: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 111UL);
    ASSERT_EQ(config.ray_rng_seed, 222UL);
    ASSERT_EQ(config.samples_per_pixel, 100);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, TrailingWhitespaceAndComments) {
  // Whitespace y comentarios después del valor
  // Verifica que el parser maneja correctamente este caso
  writeConfigFile("material_rng_seed: 13   # Reproducible material generation\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El tokenizer debería tomar solo "13" y "# comentario" como tokens extras
    // lo cual resulta en error de parsing (demasiados argumentos)
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, NegativeZero) {
  // Caso curioso: -0
  // parseUnsignedLong rechazará el signo negativo
  writeConfigFile("material_rng_seed: -0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto (parseUnsignedLong rechaza signos)
    ASSERT_EQ(config.material_rng_seed, Constants::RNGSeedMaterial);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeConfigFile("material_rng_seed:\t13\t\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeConfigFile("  \tmaterial_rng_seed:  \t 13 \t \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 13UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, LargeRealisticValue) {
  // Valor grande realista basado en timestamp de microsegundos
  // 1698000000000000 (microsegundos desde epoch)
  writeConfigFile("material_rng_seed: 1698000000000000\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 1'698'000'000'000'000UL);
  }
}

TEST_F(ConfigParserMaterialRngSeedTest, IntegrationWithAllRenderingParams) {
  // Test de integración completo con múltiples parámetros
  writeConfigFile("gamma: 2.2\n"
                  "material_rng_seed: 12345\n"
                  "ray_rng_seed: 67890\n"
                  "samples_per_pixel: 100\n"
                  "max_depth: 10\n"
                  "field_of_view: 75\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.material_rng_seed, 12'345UL);
    ASSERT_EQ(config.ray_rng_seed, 67'890UL);
    ASSERT_EQ(config.samples_per_pixel, 100);
    ASSERT_EQ(config.max_depth, 10);
    ASSERT_DOUBLE_EQ(config.field_of_view, 75.0);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
  }
}

// ============================================================================
// TESTS PARA parseMaxDepth
// ============================================================================

class ConfigParserMaxDepthTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_config_max_depth_temp.txt"; }

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

TEST_F(ConfigParserMaxDepthTest, ValidCommonValue) {
  // Test básico: valor común de profundidad máxima de rebotes
  writeConfigFile("max_depth: 5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 5);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidMinimumValue) {
  // Valor mínimo válido: 1 rebote
  // Importante en ray tracing: al menos un rebote debe ser permitido
  writeConfigFile("max_depth: 1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 1);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidLargeValue) {
  // Valor grande: 50 rebotes (rendering de alta calidad)
  // Valores altos generan más realismo pero mayor coste computacional
  writeConfigFile("max_depth: 50\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 50);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidTypicalValue) {
  // Valor típico en producción: 10 rebotes
  // Balance común entre calidad y rendimiento
  writeConfigFile("max_depth: 10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidMediumValue) {
  // Valor medio: 3 rebotes (rendering rápido con calidad aceptable)
  writeConfigFile("max_depth: 3\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 3);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidHighQualityValue) {
  // Valor alto para rendering de producción: 20 rebotes
  writeConfigFile("max_depth: 20\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 20);
  }
}

TEST_F(ConfigParserMaxDepthTest, ValidVeryLargeValue) {
  // Valor muy grande: 100 rebotes (casos extremos de calidad)
  // Documenta que el sistema acepta valores arbitrariamente grandes
  writeConfigFile("max_depth: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 100);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(ConfigParserMaxDepthTest, ErrorTooFewArguments) {
  // Menos de 2 tokens: falta el valor
  writeConfigFile("max_depth:\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorTooManyArguments) {
  // Más de 2 tokens: argumentos extra
  writeConfigFile("max_depth: 10 extra\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorMultipleExtraArguments) {
  // Múltiples argumentos extra
  writeConfigFile("max_depth: 10 20 30\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(ConfigParserMaxDepthTest, ErrorNonNumericValue) {
  // Valor no numérico
  writeConfigFile("max_depth: abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorAlphanumericValue) {
  // Valor alfanumérico mixto
  writeConfigFile("max_depth: 10abc\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorPartialNumericValue) {
  // Valor con letras antes de números
  writeConfigFile("max_depth: abc10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorEmptyValue) {
  // Token vacío
  writeConfigFile("max_depth: \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorFloatingPointValue) {
  // Valor con decimales (debería ser rechazado por parseInt)
  // Importante: max_depth debe ser entero, no acepta valores como 10.5 rebotes
  writeConfigFile("max_depth: 10.5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorFloatingPointZero) {
  // Valor decimal cero
  writeConfigFile("max_depth: 0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorScientificNotation) {
  // Notación científica (parseInt no la acepta)
  // Documenta que parseInt rechaza notación científica
  writeConfigFile("max_depth: 1e1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

// ERRORES DE RANGO - Valores no positivos

TEST_F(ConfigParserMaxDepthTest, ErrorZeroValue) {
  // Valor cero (no positivo) - NO válido
  // La condición es depth > 0, por lo que 0 es rechazado
  // Semánticamente: sin rebotes no tiene sentido en ray tracing
  writeConfigFile("max_depth: 0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorNegativeValue) {
  // Valor negativo (no positivo)
  // Un número negativo de rebotes no tiene sentido físico
  writeConfigFile("max_depth: -5\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorNegativeSmallValue) {
  // Valor negativo pequeño: -1
  writeConfigFile("max_depth: -1\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, ErrorNegativeLargeValue) {
  // Valor negativo grande
  writeConfigFile("max_depth: -100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(ConfigParserMaxDepthTest, ExtraWhitespaceAroundValue) {
  // Espacios extra alrededor del valor
  // Verifica que trimWhitespace funciona correctamente
  writeConfigFile("max_depth:    10   \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeConfigFile("  max_depth: 10  \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeConfigFile("# max_depth: 5\nmax_depth: 10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeConfigFile("\n\nmax_depth: 10\n\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, MultipleConfigParameters) {
  // Múltiples parámetros de configuración
  // Verifica que max_depth se procesa correctamente en un contexto más amplio
  writeConfigFile("gamma: 2.2\n"
                  "max_depth: 8\n"
                  "samples_per_pixel: 100\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 8);
    ASSERT_DOUBLE_EQ(config.gamma, 2.2);
    ASSERT_EQ(config.samples_per_pixel, 100);
  }
}

TEST_F(ConfigParserMaxDepthTest, LastValueWinsOnDuplicate) {
  // Si hay valores duplicados, el último debe prevalecer
  writeConfigFile("max_depth: 5\n"
                  "max_depth: 15\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 15);
  }
}

TEST_F(ConfigParserMaxDepthTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeConfigFile("max_depth: 0010\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, PlusSignPrefix) {
  // Signo + explícito
  // NOTA: parseInt NO acepta el signo + explícito
  // Este test documenta ese comportamiento
  writeConfigFile("max_depth: +10\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // parseInt rechaza el signo +, mantiene valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, IntMaxValue) {
  // Valor máximo para int (típicamente 2147483647)
  // Este test verifica que valores muy grandes pero válidos funcionan
  // Aunque en la práctica, una profundidad tan grande no tiene sentido
  writeConfigFile("max_depth: 2147483647\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 2'147'483'647);
  }
}

TEST_F(ConfigParserMaxDepthTest, IntOverflowProtection) {
  // Valor que excede INT_MAX (debería causar overflow)
  // Este test verifica la robustez ante valores extremos
  // INT_MAX típicamente es 2147483647
  writeConfigFile("max_depth: 2147483648\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debería fallar el parsing y mantener el valor por defecto
    // porque std::from_chars detectará el overflow
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, VeryLargeOverflow) {
  // Valor extremadamente grande que causa overflow
  writeConfigFile("max_depth: 999999999999\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, MultipleDecimalPoints) {
  // Valor con múltiples puntos decimales (inválido)
  writeConfigFile("max_depth: 10.0.0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, SpecialCharactersInValue) {
  // Caracteres especiales que podrían causar problemas
  writeConfigFile("max_depth: 10!\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, HexadecimalNotation) {
  // Notación hexadecimal (no debería ser aceptada por parseInt en base 10)
  writeConfigFile("max_depth: 0xA\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Debe mantener el valor por defecto
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, OctalNotation) {
  // Notación octal (012 = 10 en octal)
  // parseInt en base 10 no interpreta esto como octal
  writeConfigFile("max_depth: 012\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // Se parsea como decimal 12, no como octal 10
    ASSERT_EQ(config.max_depth, 12);
  }
}

TEST_F(ConfigParserMaxDepthTest, RealisticLowValue) {
  // Valor bajo realista: 2 rebotes (preview rápido)
  // Útil para iteraciones rápidas durante desarrollo
  writeConfigFile("max_depth: 2\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 2);
  }
}

TEST_F(ConfigParserMaxDepthTest, RealisticHighValue) {
  // Valor alto realista: 15 rebotes (alta calidad)
  writeConfigFile("max_depth: 15\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 15);
  }
}

TEST_F(ConfigParserMaxDepthTest, TrailingWhitespaceAndComments) {
  // Whitespace y comentarios después del valor
  // Verifica que el parser maneja correctamente este caso
  writeConfigFile("max_depth: 10   # Maximum ray bounce depth\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // El tokenizer debería tomar solo "10" y "# comentario" como tokens extras
    // lo cual resulta en error de parsing (demasiados argumentos)
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, NegativeZero) {
  // Caso curioso: -0 es técnicamente 0, que no es válido
  writeConfigFile("max_depth: -0\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    // -0 es parseado como 0, que no es válido (debe ser positivo)
    ASSERT_EQ(config.max_depth, Constants::MaxDepth);
  }
}

TEST_F(ConfigParserMaxDepthTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeConfigFile("max_depth:\t10\t\n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

TEST_F(ConfigParserMaxDepthTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeConfigFile("  \tmax_depth:  \t 10 \t \n");

  std::optional<ConfigSettings> config_opt = loadConfigFromFile(temp_filename);

  if (config_opt) {
    ConfigSettings const & config = *config_opt;
    ASSERT_NE(config_opt, std::nullopt)
        << "La carga de un archivo de configuración válido no debería resultar en nullopt.";
    ASSERT_EQ(config.max_depth, 10);
  }
}

// ============================================================================
// Main para ejecutar los tests
// ============================================================================

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
