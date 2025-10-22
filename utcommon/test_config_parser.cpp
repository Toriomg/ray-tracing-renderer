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
// Main para ejecutar los tests
// ============================================================================

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
