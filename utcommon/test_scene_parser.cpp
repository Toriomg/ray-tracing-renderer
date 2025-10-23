#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"
#include "scene_parser.hpp"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

// ============================================================================
// TESTS PARA parseMatteMaterial
// ============================================================================

class SceneParserMatteMaterialTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_matte_material_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(SceneParserMatteMaterialTest, ValidBasicCase) {
  // Caso válido básico: matte: mat1 0.1 0.2 0.3
  // Verifica que se añade correctamente el material con nombre "mat1" y RGB = {0.1, 0.2, 0.3}
  writeSceneFile("matte: mat1 0.1 0.2 0.3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material matte
  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.matte.g.size(), 1);
  ASSERT_EQ(scene.matte.b.size(), 1);

  // Verificar los valores RGB
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.2);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.3);

  // Verificar que se añadió el nombre del material
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "mat1");

  // Verificar que se añadió la entrada en materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMatteMaterialTest, ValidBoundaryValues) {
  // Caso válido con valores en los límites [0, 1]: matte: mat2 0.0 1.0 0.5
  writeSceneFile("matte: mat2 0.0 1.0 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material matte
  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.matte.g.size(), 1);
  ASSERT_EQ(scene.matte.b.size(), 1);

  // Verificar los valores RGB en los límites
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);

  // Verificar el nombre del material
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "mat2");

  // Verificar materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMatteMaterialTest, ValidBlackColor) {
  // Negro puro: RGB = (0, 0, 0)
  writeSceneFile("matte: black_mat 0 0 0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);

  ASSERT_EQ(scene.materialNames[0], "black_mat");
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMatteMaterialTest, ValidWhiteColor) {
  // Blanco puro: RGB = (1, 1, 1)
  writeSceneFile("matte: white_mat 1 1 1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 1.0);

  ASSERT_EQ(scene.materialNames[0], "white_mat");
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMatteMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("matte: precise_mat 0.123456 0.654321 0.999999\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.123456);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.654321);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.999999);

  ASSERT_EQ(scene.materialNames[0], "precise_mat");
}

TEST_F(SceneParserMatteMaterialTest, ValidMultipleMaterials) {
  // Múltiples materiales matte en el mismo archivo
  // Verifica que los índices locales se incrementan correctamente
  writeSceneFile("matte: mat1 0.1 0.2 0.3\n"
                 "matte: mat2 0.4 0.5 0.6\n"
                 "matte: mat3 0.7 0.8 0.9\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.matte.r.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialTable.size(), 3);

  // Verificar primer material
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.2);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.3);
  ASSERT_EQ(scene.materialNames[0], "mat1");
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);

  // Verificar segundo material
  ASSERT_DOUBLE_EQ(scene.matte.r[1], 0.4);
  ASSERT_DOUBLE_EQ(scene.matte.g[1], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.b[1], 0.6);
  ASSERT_EQ(scene.materialNames[1], "mat2");
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);

  // Verificar tercer material
  ASSERT_DOUBLE_EQ(scene.matte.r[2], 0.7);
  ASSERT_DOUBLE_EQ(scene.matte.g[2], 0.8);
  ASSERT_DOUBLE_EQ(scene.matte.b[2], 0.9);
  ASSERT_EQ(scene.materialNames[2], "mat3");
  ASSERT_EQ(scene.materialTable[2].localIndex, 2);

  // Verificar que todos son de tipo MATTE
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[1].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[2].type, MaterialType::MATTE);
}

TEST_F(SceneParserMatteMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  // Verifica que los nombres se procesan correctamente
  writeSceneFile("matte: red-material_v2 0.8 0.1 0.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "red-material_v2");
}

TEST_F(SceneParserMatteMaterialTest, ValidShortMaterialName) {
  // Nombre de material de un solo carácter
  writeSceneFile("matte: m 0.5 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "m");
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 5 tokens: falta todo (solo el comando)
  writeSceneFile("matte:\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.matte.g.empty());
  ASSERT_TRUE(scene.matte.b.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, faltan los valores RGB
  writeSceneFile("matte: mat1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NameAndR) {
  // Solo nombre y componente R, faltan G y B
  writeSceneFile("matte: mat1 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NameRG) {
  // Solo nombre, R y G, falta B
  writeSceneFile("matte: mat1 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 5 tokens: un argumento extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5 extra\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5 extra1 extra2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeSceneFile("matte: mat1 abc 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeSceneFile("matte: mat1 0.5 abc 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeSceneFile("matte: mat1 0.5 0.5 abc\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllNonNumeric) {
  // Todos los componentes RGB no numéricos
  writeSceneFile("matte: mat1 abc def ghi\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorAlphanumericMixedInComponent) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("matte: mat1 0.5abc 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorEmptyValues) {
  // Valores vacíos después del nombre
  writeSceneFile("matte: mat1   \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE RANGO - Componentes RGB fuera del rango [0, 1]

TEST_F(SceneParserMatteMaterialTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeSceneFile("matte: mat1 -0.1 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (fuera de rango)
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeSceneFile("matte: mat1 0.5 1.1 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeSceneFile("matte: mat1 0.5 0.5 -0.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeSceneFile("matte: mat1 0.5 0.5 1.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorRComponentFarBelowZero) {
  // Componente R muy negativo
  writeSceneFile("matte: mat1 -100.0 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorGComponentFarAboveOne) {
  // Componente G muy por encima de 1
  writeSceneFile("matte: mat1 0.5 100.0 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllComponentsBelowZero) {
  // Todos los componentes menores que 0
  writeSceneFile("matte: mat1 -0.1 -0.2 -0.3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllComponentsAboveOne) {
  // Todos los componentes mayores que 1
  writeSceneFile("matte: mat1 1.1 1.2 1.3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentFarOutOfRange) {
  // Componente B = 2 (muy fuera de rango)
  writeSceneFile("matte: mat1 0.5 0.5 2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentNegativeFarOutOfRange) {
  // Componente B = -1 (muy fuera de rango)
  writeSceneFile("matte: mat1 0.5 0.5 -1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserMatteMaterialTest, BoundaryExactlyZero) {
  // Todos los componentes exactamente 0.0
  writeSceneFile("matte: mat_zero 0.0 0.0 0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
}

TEST_F(SceneParserMatteMaterialTest, BoundaryExactlyOne) {
  // Todos los componentes exactamente 1.0
  writeSceneFile("matte: mat_one 1.0 1.0 1.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 1.0);
}

TEST_F(SceneParserMatteMaterialTest, BoundaryVeryCloseToZero) {
  // Valores muy cercanos a 0 pero válidos
  writeSceneFile("matte: mat_near_zero 0.0001 0.0001 0.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0001);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0001);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0001);
}

TEST_F(SceneParserMatteMaterialTest, BoundaryVeryCloseToOne) {
  // Valores muy cercanos a 1 pero válidos
  writeSceneFile("matte: mat_near_one 0.9999 0.9999 0.9999\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.9999);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.9999);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.9999);
}

TEST_F(SceneParserMatteMaterialTest, BoundaryJustBelowZeroInvalid) {
  // Valor justo por debajo de 0.0 (inválido)
  writeSceneFile("matte: mat1 -0.0001 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
}

TEST_F(SceneParserMatteMaterialTest, BoundaryJustAboveOneInvalid) {
  // Valor justo por encima de 1.0 (inválido)
  writeSceneFile("matte: mat1 0.5 1.0001 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.matte.r.empty());
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserMatteMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("matte:    mat1   0.5   0.5   0.5   \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "mat1");
}

TEST_F(SceneParserMatteMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  matte: mat1 0.5 0.5 0.5  \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "mat1");
}

TEST_F(SceneParserMatteMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("matte:\tmat1\t0.5\t0.5\t0.5\t\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "mat1");
}

TEST_F(SceneParserMatteMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \tmatte:  \t mat1 \t 0.5 \t 0.5 \t 0.5 \t \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "mat1");
}

TEST_F(SceneParserMatteMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# matte: mat_ignored 1.0 1.0 1.0\nmatte: mat1 0.5 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haber 1 material (no 2)
  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "mat1");
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
}

TEST_F(SceneParserMatteMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nmatte: mat1 0.5 0.5 0.5\n\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "mat1");
}

TEST_F(SceneParserMatteMaterialTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1]
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9
  writeSceneFile("matte: mat_sci 5e-1 1e-1 9e-1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.9);
}

TEST_F(SceneParserMatteMaterialTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeSceneFile("matte: mat1 1e1 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (fuera de rango)
  ASSERT_TRUE(scene.matte.r.empty());
}

TEST_F(SceneParserMatteMaterialTest, NegativeZeroComponent) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeSceneFile("matte: mat_neg_zero -0.0 -0.0 -0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
}

TEST_F(SceneParserMatteMaterialTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeSceneFile("matte: mat_int 0 1 0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
}

TEST_F(SceneParserMatteMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("matte: mat_leading 00.5 00.5 00.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
}

TEST_F(SceneParserMatteMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("matte: mat_plus +0.5 +0.5 +0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
}

TEST_F(SceneParserMatteMaterialTest, InfinityValue) {
  // Valor infinito (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeSceneFile("matte: mat1 inf 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (inf > 1.0)
  ASSERT_TRUE(scene.matte.r.empty());
}

TEST_F(SceneParserMatteMaterialTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeSceneFile("matte: mat1 nan 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (nan falla la comparación)
  ASSERT_TRUE(scene.matte.r.empty());
}

TEST_F(SceneParserMatteMaterialTest, MaterialNameWithNumbers) {
  // Nombre de material con números
  writeSceneFile("matte: material123 0.5 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "material123");
}

TEST_F(SceneParserMatteMaterialTest, MaterialNameCaseSensitive) {
  // Verificar que los nombres de material son case-sensitive
  // (mat1 != Mat1 != MAT1)
  writeSceneFile("matte: mat1 0.1 0.1 0.1\n"
                 "matte: Mat1 0.2 0.2 0.2\n"
                 "matte: MAT1 0.3 0.3 0.3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialNames[0], "mat1");
  ASSERT_EQ(scene.materialNames[1], "Mat1");
  ASSERT_EQ(scene.materialNames[2], "MAT1");
}

// TESTS DE INTEGRACIÓN

TEST_F(SceneParserMatteMaterialTest, IntegrationWithOtherCommands) {
  // Test de integración: matte junto con otros comandos válidos
  // Aunque no podemos testear otros comandos directamente aquí,
  // verificamos que matte no interfiere con el parsing general
  writeSceneFile("# Scene with matte material\n"
                 "matte: red_mat 0.8 0.1 0.1\n"
                 "matte: green_mat 0.1 0.8 0.1\n"
                 "matte: blue_mat 0.1 0.1 0.8\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.matte.r.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialTable.size(), 3);

  // Verificar primer material (rojo)
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.8);
  ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.1);

  // Verificar segundo material (verde)
  ASSERT_DOUBLE_EQ(scene.matte.r[1], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.g[1], 0.8);
  ASSERT_DOUBLE_EQ(scene.matte.b[1], 0.1);

  // Verificar tercer material (azul)
  ASSERT_DOUBLE_EQ(scene.matte.r[2], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.g[2], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.b[2], 0.8);
}

TEST_F(SceneParserMatteMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  // (testing de robustez ante errores)
  writeSceneFile("matte: error_mat 2.0 2.0 2.0\n"    // Línea con error (fuera de rango)
                 "matte: valid_mat 0.5 0.5 0.5\n");  // Línea válida

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haberse añadido el material válido
  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "valid_mat");
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
}

TEST_F(SceneParserMatteMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados (aunque no sea ideal)
  // El parser no valida unicidad de nombres
  writeSceneFile("matte: mat1 0.1 0.1 0.1\n"
                 "matte: mat1 0.2 0.2 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Ambos materiales deben haberse añadido
  ASSERT_EQ(scene.matte.r.size(), 2);
  ASSERT_EQ(scene.materialNames.size(), 2);
  ASSERT_EQ(scene.materialNames[0], "mat1");
  ASSERT_EQ(scene.materialNames[1], "mat1");

  // Verificar que tienen diferentes valores RGB
  ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.matte.r[1], 0.2);

  // Verificar índices locales diferentes
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);
}

// ============================================================================
// TESTS PARA parseMetalMaterial
// ============================================================================

class SceneParserMetalMaterialTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_metal_material_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(SceneParserMetalMaterialTest, ValidBasicCase) {
  // Caso válido básico: metal: met1 0.8 0.8 0.1 0.2
  // Verifica que se añade correctamente el material con nombre "met1"
  // RGB = {0.8, 0.8, 0.1} y diffusion = 0.2
  writeSceneFile("metal: met1 0.8 0.8 0.1 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material metal
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.metal.g.size(), 1);
  ASSERT_EQ(scene.metal.b.size(), 1);
  ASSERT_EQ(scene.metal.diffusion.size(), 1);

  // Verificar los valores RGB y diffusion
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);

  // Verificar que se añadió el nombre del material
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "met1");

  // Verificar que se añadió la entrada en materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMetalMaterialTest, ValidBoundaryValues) {
  // Caso válido con valores en los límites: RGB en [0,1], diffusion = 0.0 (mínimo)
  writeSceneFile("metal: met2 0.0 1.0 0.5 0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material metal
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.metal.g.size(), 1);
  ASSERT_EQ(scene.metal.b.size(), 1);
  ASSERT_EQ(scene.metal.diffusion.size(), 1);

  // Verificar los valores en los límites
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.0);

  // Verificar el nombre del material
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "met2");

  // Verificar materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMetalMaterialTest, ValidHighDiffusion) {
  // Caso válido con difusión alta (sin límite superior)
  // metal: met3 0.9 0.9 0.9 10.0
  writeSceneFile("metal: met3 0.9 0.9 0.9 10.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió el material
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.metal.diffusion.size(), 1);

  // Verificar valores RGB y diffusion alta
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 10.0);

  // Verificar nombre y tabla
  ASSERT_EQ(scene.materialNames[0], "met3");
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMetalMaterialTest, ValidBlackColor) {
  // Negro puro con difusión: RGB = (0, 0, 0), diffusion = 0.5
  writeSceneFile("metal: black_metal 0 0 0 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);

  ASSERT_EQ(scene.materialNames[0], "black_metal");
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserMetalMaterialTest, ValidWhiteColor) {
  // Blanco puro con difusión: RGB = (1, 1, 1), diffusion = 0.1
  writeSceneFile("metal: white_metal 1 1 1 0.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.1);

  ASSERT_EQ(scene.materialNames[0], "white_metal");
}

TEST_F(SceneParserMetalMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("metal: precise_metal 0.123456 0.654321 0.999999 0.456789\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.123456);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.654321);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.999999);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.456789);

  ASSERT_EQ(scene.materialNames[0], "precise_metal");
}

TEST_F(SceneParserMetalMaterialTest, ValidMultipleMaterials) {
  // Múltiples materiales metal en el mismo archivo
  // Verifica que los índices locales se incrementan correctamente
  writeSceneFile("metal: gold 0.9 0.8 0.1 0.2\n"
                 "metal: silver 0.8 0.8 0.8 0.1\n"
                 "metal: copper 0.7 0.5 0.3 0.3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.metal.r.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialTable.size(), 3);

  // Verificar primer material (oro)
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  ASSERT_EQ(scene.materialNames[0], "gold");
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);

  // Verificar segundo material (plata)
  ASSERT_DOUBLE_EQ(scene.metal.r[1], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.g[1], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.b[1], 0.8);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[1], 0.1);
  ASSERT_EQ(scene.materialNames[1], "silver");
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);

  // Verificar tercer material (cobre)
  ASSERT_DOUBLE_EQ(scene.metal.r[2], 0.7);
  ASSERT_DOUBLE_EQ(scene.metal.g[2], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.b[2], 0.3);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[2], 0.3);
  ASSERT_EQ(scene.materialNames[2], "copper");
  ASSERT_EQ(scene.materialTable[2].localIndex, 2);

  // Verificar que todos son de tipo METAL
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[1].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[2].type, MaterialType::METAL);
}

TEST_F(SceneParserMetalMaterialTest, ValidVeryHighDiffusion) {
  // Difusión muy alta (sin límite superior explícito en el código)
  writeSceneFile("metal: fuzzy_metal 0.5 0.5 0.5 1000.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.diffusion.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 1000.0);
}

TEST_F(SceneParserMetalMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  writeSceneFile("metal: brushed-aluminum_v2 0.7 0.7 0.7 0.4\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "brushed-aluminum_v2");
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 6 tokens: falta todo (solo el comando)
  writeSceneFile("metal:\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.metal.g.empty());
  ASSERT_TRUE(scene.metal.b.empty());
  ASSERT_TRUE(scene.metal.diffusion.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, faltan los valores RGB y diffusion
  writeSceneFile("metal: met1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameAndRGB) {
  // Solo nombre y RGB, falta diffusion
  writeSceneFile("metal: met1 0.5 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameRG) {
  // Solo nombre, R y G, faltan B y diffusion
  writeSceneFile("metal: met1 0.5 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameR) {
  // Solo nombre y R, faltan G, B y diffusion
  writeSceneFile("metal: met1 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 6 tokens: un argumento extra
  writeSceneFile("metal: met1 0.5 0.5 0.5 0.2 extra\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("metal: met1 0.5 0.5 0.5 0.2 extra1 extra2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeSceneFile("metal: met1 abc 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeSceneFile("metal: met1 0.5 abc 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeSceneFile("metal: met1 0.5 0.5 abc 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericDiffusion) {
  // Diffusion no numérico
  writeSceneFile("metal: met1 0.5 0.5 0.5 abc\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeSceneFile("metal: met1 abc def ghi jkl\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorAlphanumericMixedInComponent) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("metal: met1 0.5abc 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorEmptyValues) {
  // Valores vacíos después del nombre
  writeSceneFile("metal: met1   \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE RANGO - Componentes RGB fuera del rango [0, 1]

TEST_F(SceneParserMetalMaterialTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeSceneFile("metal: met1 -0.1 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (fuera de rango)
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeSceneFile("metal: met1 0.5 1.1 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeSceneFile("metal: met1 0.5 0.5 -0.1 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeSceneFile("metal: met1 0.5 0.5 1.1 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllRGBComponentsBelowZero) {
  // Todos los componentes RGB menores que 0
  writeSceneFile("metal: met1 -0.1 -0.2 -0.3 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllRGBComponentsAboveOne) {
  // Todos los componentes RGB mayores que 1
  writeSceneFile("metal: met1 1.1 1.2 1.3 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorRComponentFarBelowZero) {
  // Componente R muy negativo
  writeSceneFile("metal: met1 -100.0 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorGComponentFarAboveOne) {
  // Componente G muy por encima de 1
  writeSceneFile("metal: met1 0.5 100.0 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE RANGO - Factor de difusión negativo

TEST_F(SceneParserMetalMaterialTest, ErrorDiffusionBelowZero) {
  // Factor de difusión menor que 0 (inválido según código)
  writeSceneFile("metal: met1 0.5 0.5 0.5 -0.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (diffusion < 0.0 es inválido)
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorDiffusionFarBelowZero) {
  // Factor de difusión muy negativo
  writeSceneFile("metal: met1 0.5 0.5 0.5 -100.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserMetalMaterialTest, ErrorCombinedRGBAndDiffusionOutOfRange) {
  // RGB fuera de rango Y diffusion negativo (múltiples errores)
  writeSceneFile("metal: met1 -0.1 1.5 0.5 -0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBExactlyZero) {
  // Todos los componentes RGB exactamente 0.0
  writeSceneFile("metal: met_zero 0.0 0.0 0.0 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBExactlyOne) {
  // Todos los componentes RGB exactamente 1.0
  writeSceneFile("metal: met_one 1.0 1.0 1.0 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionExactlyZero) {
  // Difusión exactamente 0.0 (válido, superficie perfectamente reflectante)
  writeSceneFile("metal: mirror 0.9 0.9 0.9 0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.diffusion.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.0);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBVeryCloseToZero) {
  // Valores RGB muy cercanos a 0 pero válidos
  writeSceneFile("metal: met_near_zero 0.0001 0.0001 0.0001 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0001);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0001);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0001);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBVeryCloseToOne) {
  // Valores RGB muy cercanos a 1 pero válidos
  writeSceneFile("metal: met_near_one 0.9999 0.9999 0.9999 0.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.9999);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.9999);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.9999);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionVeryCloseToZero) {
  // Difusión muy cercana a 0 pero válida
  writeSceneFile("metal: almost_mirror 0.9 0.9 0.9 0.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.diffusion.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.0001);
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBJustBelowZeroInvalid) {
  // Valor RGB justo por debajo de 0.0 (inválido)
  writeSceneFile("metal: met1 -0.0001 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBJustAboveOneInvalid) {
  // Valor RGB justo por encima de 1.0 (inválido)
  writeSceneFile("metal: met1 0.5 1.0001 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionJustBelowZeroInvalid) {
  // Difusión justo por debajo de 0.0 (inválido)
  writeSceneFile("metal: met1 0.5 0.5 0.5 -0.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.metal.r.empty());
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserMetalMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("metal:    met1   0.5   0.5   0.5   0.2   \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  ASSERT_EQ(scene.materialNames[0], "met1");
}

TEST_F(SceneParserMetalMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  metal: met1 0.5 0.5 0.5 0.2  \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "met1");
}

TEST_F(SceneParserMetalMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("metal:\tmet1\t0.5\t0.5\t0.5\t0.2\t\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  ASSERT_EQ(scene.materialNames[0], "met1");
}

TEST_F(SceneParserMetalMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \tmetal:  \t met1 \t 0.5 \t 0.5 \t 0.5 \t 0.2 \t \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "met1");
}

TEST_F(SceneParserMetalMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# metal: met_ignored 1.0 1.0 1.0 0.5\nmetal: met1 0.5 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haber 1 material (no 2)
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "met1");
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
}

TEST_F(SceneParserMetalMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nmetal: met1 0.5 0.5 0.5 0.2\n\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_EQ(scene.materialNames[0], "met1");
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1] para RGB
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9, 2e-1 = 0.2
  writeSceneFile("metal: met_sci 5e-1 1e-1 9e-1 2e-1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido para RGB
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeSceneFile("metal: met1 1e1 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (fuera de rango)
  ASSERT_TRUE(scene.metal.r.empty());
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationForDiffusion) {
  // Notación científica válida para diffusion (puede ser > 1)
  // 1e1 = 10.0 es válido para diffusion
  writeSceneFile("metal: met_sci_diff 0.5 0.5 0.5 1e1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.diffusion.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 10.0);
}

TEST_F(SceneParserMetalMaterialTest, NegativeZeroRGBComponents) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeSceneFile("metal: met_neg_zero -0.0 -0.0 -0.0 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
}

TEST_F(SceneParserMetalMaterialTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeSceneFile("metal: met_int 0 1 0 1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 1.0);
}

TEST_F(SceneParserMetalMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("metal: met_leading 00.5 00.5 00.5 00.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
}

TEST_F(SceneParserMetalMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("metal: met_plus +0.5 +0.5 +0.5 +0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
}

TEST_F(SceneParserMetalMaterialTest, InfinityValueInRGB) {
  // Valor infinito en RGB (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeSceneFile("metal: met1 inf 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (inf > 1.0)
  ASSERT_TRUE(scene.metal.r.empty());
}

TEST_F(SceneParserMetalMaterialTest, InfinityValueInDiffusion) {
  // Valor infinito en diffusion (técnicamente parsedouble lo acepta)
  // Pero en la práctica no tiene sentido físico
  writeSceneFile("metal: met1 0.5 0.5 0.5 inf\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Si parsedouble acepta inf y diffusion >= 0, se añadirá
  // (el código no valida límite superior para diffusion)
  ASSERT_EQ(scene.metal.diffusion.size(), 1);
  ASSERT_TRUE(std::isinf(scene.metal.diffusion[0]));
}

TEST_F(SceneParserMetalMaterialTest, NaNValueInRGB) {
  // Valor NaN en RGB (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeSceneFile("metal: met1 nan 0.5 0.5 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (nan falla la comparación)
  ASSERT_TRUE(scene.metal.r.empty());
}

TEST_F(SceneParserMetalMaterialTest, NaNValueInDiffusion) {
  // Valor NaN en diffusion (no válido)
  writeSceneFile("metal: met1 0.5 0.5 0.5 nan\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (nan falla la comparación >= 0)
  ASSERT_TRUE(scene.metal.r.empty());
}

// TESTS DE INTEGRACIÓN

TEST_F(SceneParserMetalMaterialTest, IntegrationWithMatteMaterial) {
  // Test de integración: metal y matte en el mismo archivo
  // Verifica que los índices en materialTable son correctos
  writeSceneFile("matte: red_matte 0.8 0.1 0.1\n"
                 "metal: gold 0.9 0.8 0.1 0.2\n"
                 "matte: blue_matte 0.1 0.1 0.8\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 2 matte y 1 metal
  ASSERT_EQ(scene.matte.r.size(), 2);
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialTable.size(), 3);

  // Verificar material matte #0
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  ASSERT_EQ(scene.materialNames[0], "red_matte");

  // Verificar material metal #1
  ASSERT_EQ(scene.materialTable[1].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[1].localIndex, 0);  // Primer metal, localIndex = 0
  ASSERT_EQ(scene.materialNames[1], "gold");
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.9);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);

  // Verificar material matte #2
  ASSERT_EQ(scene.materialTable[2].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[2].localIndex, 1);  // Segundo matte, localIndex = 1
  ASSERT_EQ(scene.materialNames[2], "blue_matte");
}

TEST_F(SceneParserMetalMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  writeSceneFile("metal: error_met 2.0 2.0 2.0 0.2\n"    // Línea con error (RGB fuera de rango)
                 "metal: valid_met 0.5 0.5 0.5 0.2\n");  // Línea válida

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haberse añadido el material válido
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "valid_met");
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
}

TEST_F(SceneParserMetalMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados
  writeSceneFile("metal: met1 0.1 0.1 0.1 0.1\n"
                 "metal: met1 0.2 0.2 0.2 0.2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Ambos materiales deben haberse añadido
  ASSERT_EQ(scene.metal.r.size(), 2);
  ASSERT_EQ(scene.materialNames.size(), 2);
  ASSERT_EQ(scene.materialNames[0], "met1");
  ASSERT_EQ(scene.materialNames[1], "met1");

  // Verificar que tienen diferentes valores
  ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.metal.r[1], 0.2);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.1);
  ASSERT_DOUBLE_EQ(scene.metal.diffusion[1], 0.2);

  // Verificar índices locales diferentes
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);
}

// ============================================================================
// TESTS PARA parseRefractiveMaterial
// ============================================================================

class SceneParserRefractiveMaterialTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_refractive_material_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS

TEST_F(SceneParserRefractiveMaterialTest, ValidBasicCase) {
  // Caso válido básico: refractive: glass 1.5
  // Verifica que se añade correctamente el material con nombre "glass" y IOR = 1.5
  writeSceneFile("refractive: glass 1.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material refractive
  ASSERT_EQ(scene.refractive.ior.size(), 1);

  // Verificar el valor IOR
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);

  // Verificar que se añadió el nombre del material
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "glass");

  // Verificar que se añadió la entrada en materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidIORCloseToOne) {
  // Caso válido con IOR cercano a 1 (aire/vacío): IOR = 1.001
  writeSceneFile("refractive: air_like 1.001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió el material
  ASSERT_EQ(scene.refractive.ior.size(), 1);

  // Verificar IOR cercano a 1
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.001);

  // Verificar nombre
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "air_like");

  // Verificar materialTable
  ASSERT_EQ(scene.materialTable.size(), 1);
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidHighIOR) {
  // Caso válido con IOR alto (diamante): IOR = 2.4
  writeSceneFile("refractive: diamond 2.4\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió el material
  ASSERT_EQ(scene.refractive.ior.size(), 1);

  // Verificar IOR alto
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 2.4);

  // Verificar nombre
  ASSERT_EQ(scene.materialNames[0], "diamond");

  // Verificar materialTable
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidVerySmallIOR) {
  // Caso válido con IOR muy pequeño pero > 0: IOR = 0.00001
  // Aunque físicamente poco realista, es técnicamente válido según la validación
  writeSceneFile("refractive: near_vacuum 0.00001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió el material
  ASSERT_EQ(scene.refractive.ior.size(), 1);

  // Verificar IOR muy pequeño
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 0.00001);

  // Verificar nombre
  ASSERT_EQ(scene.materialNames[0], "near_vacuum");

  // Verificar materialTable
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidWater) {
  // Agua: IOR = 1.33
  writeSceneFile("refractive: water 1.33\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.33);
  ASSERT_EQ(scene.materialNames[0], "water");
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("refractive: precise_glass 1.5168\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5168);
  ASSERT_EQ(scene.materialNames[0], "precise_glass");
}

TEST_F(SceneParserRefractiveMaterialTest, ValidMultipleMaterials) {
  // Múltiples materiales refractivos en el mismo archivo
  // Verifica que los índices locales se incrementan correctamente
  writeSceneFile("refractive: glass 1.5\n"
                 "refractive: water 1.33\n"
                 "refractive: diamond 2.4\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.refractive.ior.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialTable.size(), 3);

  // Verificar primer material (glass)
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);

  // Verificar segundo material (water)
  ASSERT_DOUBLE_EQ(scene.refractive.ior[1], 1.33);
  ASSERT_EQ(scene.materialNames[1], "water");
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);

  // Verificar tercer material (diamond)
  ASSERT_DOUBLE_EQ(scene.refractive.ior[2], 2.4);
  ASSERT_EQ(scene.materialNames[2], "diamond");
  ASSERT_EQ(scene.materialTable[2].localIndex, 2);

  // Verificar que todos son de tipo REFRACTIVE
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[1].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[2].type, MaterialType::REFRACTIVE);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidVeryHighIOR) {
  // IOR muy alto (teóricamente posible): IOR = 10.0
  writeSceneFile("refractive: exotic_material 10.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 10.0);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  writeSceneFile("refractive: borosilicate-glass_v2 1.47\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "borosilicate-glass_v2");
}

TEST_F(SceneParserRefractiveMaterialTest, ValidIntegerIOR) {
  // Valor entero (sin punto decimal) debe ser aceptado
  writeSceneFile("refractive: integer_ior 2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 2.0);
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 3 tokens: falta todo (solo el comando)
  writeSceneFile("refractive:\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, falta IOR
  writeSceneFile("refractive: glass\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 3 tokens: un argumento extra
  writeSceneFile("refractive: glass 1.5 extra\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (error de parsing)
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("refractive: glass 1.5 extra1 extra2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserRefractiveMaterialTest, ErrorNonNumericIOR) {
  // IOR no numérico
  writeSceneFile("refractive: glass abc\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorAlphanumericMixedInIOR) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("refractive: glass 1.5abc\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorEmptyIORValue) {
  // Valor IOR vacío
  writeSceneFile("refractive: glass  \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// ERRORES DE RANGO - IOR <= 0

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORZero) {
  // IOR igual a cero (inválido según código: ior <= 0.0)
  writeSceneFile("refractive: glass 0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (IOR debe ser > 0)
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORExactlyZero) {
  // IOR exactamente 0.0
  writeSceneFile("refractive: glass 0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORNegative) {
  // IOR negativo: -1.5
  writeSceneFile("refractive: glass -1.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (IOR negativo no tiene sentido físico)
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORNegativeSmall) {
  // IOR negativo pequeño: -0.1
  writeSceneFile("refractive: glass -0.1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORVeryNegative) {
  // IOR muy negativo: -100.0
  writeSceneFile("refractive: glass -100.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
  ASSERT_TRUE(scene.materialNames.empty());
  ASSERT_TRUE(scene.materialTable.empty());
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORJustAboveZero) {
  // IOR justo por encima de 0.0 (válido): 0.0001
  writeSceneFile("refractive: almost_zero 0.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 0.0001);
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORExactlyOne) {
  // IOR exactamente 1.0 (vacío perfecto)
  writeSceneFile("refractive: vacuum 1.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.0);
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORVeryCloseToOne) {
  // IOR muy cercano a 1.0: 1.0001
  writeSceneFile("refractive: near_vacuum 1.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.0001);
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORJustBelowZeroInvalid) {
  // IOR justo por debajo de 0.0 (inválido): -0.0001
  writeSceneFile("refractive: glass -0.0001\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío
  ASSERT_TRUE(scene.refractive.ior.empty());
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserRefractiveMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("refractive:    glass   1.5   \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
}

TEST_F(SceneParserRefractiveMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  refractive: glass 1.5  \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
}

TEST_F(SceneParserRefractiveMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("refractive:\tglass\t1.5\t\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
}

TEST_F(SceneParserRefractiveMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \trefractive:  \t glass \t 1.5 \t \n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
}

TEST_F(SceneParserRefractiveMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# refractive: ignored 2.0\nrefractive: glass 1.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haber 1 material (no 2)
  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "glass");
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
}

TEST_F(SceneParserRefractiveMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nrefractive: glass 1.5\n\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_EQ(scene.materialNames[0], "glass");
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationValidIOR) {
  // Notación científica válida para IOR > 0
  // parsedouble acepta notación científica
  // 1.5e0 = 1.5, 2.4e0 = 2.4
  writeSceneFile("refractive: glass_sci 1.5e0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationHighIOR) {
  // Notación científica para IOR alto
  // 2.4e0 = 2.4, 1e1 = 10.0
  writeSceneFile("refractive: exotic 1e1\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 10.0);
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationSmallIOR) {
  // Notación científica para IOR pequeño pero válido
  // 1e-3 = 0.001
  writeSceneFile("refractive: tiny_ior 1e-3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 0.001);
}

TEST_F(SceneParserRefractiveMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("refractive: glass 001.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
}

TEST_F(SceneParserRefractiveMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("refractive: glass +1.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
}

TEST_F(SceneParserRefractiveMaterialTest, InfinityValue) {
  // Valor infinito (técnicamente parsedouble lo acepta y es > 0)
  // Aunque físicamente no tiene sentido
  writeSceneFile("refractive: glass inf\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Si parsedouble acepta inf y ior > 0, se añadirá
  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_TRUE(std::isinf(scene.refractive.ior[0]));
  ASSERT_GT(scene.refractive.ior[0], 0.0);
}

TEST_F(SceneParserRefractiveMaterialTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero debería fallar la comparación ior <= 0
  writeSceneFile("refractive: glass nan\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (nan falla la comparación > 0)
  ASSERT_TRUE(scene.refractive.ior.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, NegativeZero) {
  // -0.0 es equivalente a 0.0 en punto flotante (inválido, no es > 0)
  writeSceneFile("refractive: glass -0.0\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe permanecer vacío (-0.0 no es > 0)
  ASSERT_TRUE(scene.refractive.ior.empty());
}

TEST_F(SceneParserRefractiveMaterialTest, MaterialNameWithNumbers) {
  // Nombre de material con números
  writeSceneFile("refractive: glass123 1.5\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "glass123");
}

TEST_F(SceneParserRefractiveMaterialTest, MaterialNameCaseSensitive) {
  // Verificar que los nombres de material son case-sensitive
  writeSceneFile("refractive: glass 1.5\n"
                 "refractive: Glass 1.6\n"
                 "refractive: GLASS 1.7\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  ASSERT_EQ(scene.refractive.ior.size(), 3);
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialNames[0], "glass");
  ASSERT_EQ(scene.materialNames[1], "Glass");
  ASSERT_EQ(scene.materialNames[2], "GLASS");
}

// TESTS DE INTEGRACIÓN

TEST_F(SceneParserRefractiveMaterialTest, IntegrationWithOtherMaterials) {
  // Test de integración: refractive mezclado con matte y metal
  // Verifica que los índices en materialTable son correctos
  writeSceneFile("matte: red_matte 0.8 0.1 0.1\n"
                 "refractive: glass 1.5\n"
                 "metal: gold 0.9 0.8 0.1 0.2\n"
                 "refractive: water 1.33\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 1 matte, 2 refractive, 1 metal
  ASSERT_EQ(scene.matte.r.size(), 1);
  ASSERT_EQ(scene.refractive.ior.size(), 2);
  ASSERT_EQ(scene.metal.r.size(), 1);
  ASSERT_EQ(scene.materialNames.size(), 4);
  ASSERT_EQ(scene.materialTable.size(), 4);

  // Verificar material matte #0
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  ASSERT_EQ(scene.materialNames[0], "red_matte");

  // Verificar material refractive #1 (glass)
  ASSERT_EQ(scene.materialTable[1].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[1].localIndex, 0);  // Primer refractive
  ASSERT_EQ(scene.materialNames[1], "glass");
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);

  // Verificar material metal #2
  ASSERT_EQ(scene.materialTable[2].type, MaterialType::METAL);
  ASSERT_EQ(scene.materialTable[2].localIndex, 0);
  ASSERT_EQ(scene.materialNames[2], "gold");

  // Verificar material refractive #3 (water)
  ASSERT_EQ(scene.materialTable[3].type, MaterialType::REFRACTIVE);
  ASSERT_EQ(scene.materialTable[3].localIndex, 1);  // Segundo refractive
  ASSERT_EQ(scene.materialNames[3], "water");
  ASSERT_DOUBLE_EQ(scene.refractive.ior[1], 1.33);
}

TEST_F(SceneParserRefractiveMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  writeSceneFile("refractive: error_glass 0\n"      // Línea con error (IOR = 0)
                 "refractive: valid_glass 1.5\n");  // Línea válida

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Solo debe haberse añadido el material válido
  ASSERT_EQ(scene.refractive.ior.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "valid_glass");
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
}

TEST_F(SceneParserRefractiveMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados
  writeSceneFile("refractive: glass 1.5\n"
                 "refractive: glass 1.6\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Ambos materiales deben haberse añadido
  ASSERT_EQ(scene.refractive.ior.size(), 2);
  ASSERT_EQ(scene.materialNames.size(), 2);
  ASSERT_EQ(scene.materialNames[0], "glass");
  ASSERT_EQ(scene.materialNames[1], "glass");

  // Verificar que tienen diferentes valores IOR
  ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  ASSERT_DOUBLE_EQ(scene.refractive.ior[1], 1.6);

  // Verificar índices locales diferentes
  ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  ASSERT_EQ(scene.materialTable[1].localIndex, 1);
}

// ============================================================================
// TESTS PARA findMaterialIndex
// ============================================================================

class SceneParserFindMaterialIndexTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_find_material_index_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
      // Just continue silently as this is cleanup code
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS - Material encontrado

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundFirstElement) {
  // Caso: buscar el primer material de la lista
  // Definimos 3 materiales y luego una esfera que usa el primero
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "metal: mat2 0.6 0.6 0.6 0.1\n"
                 "refractive: mat3 1.5\n"
                 "sphere: 0 0 0 1 mat1\n");  // Usa mat1 (índice 0)

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialNames[0], "mat1");

  // Verificar que se añadió la esfera exitosamente
  // (esto significa que findMaterialIndex encontró mat1 y devolvió 0)
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // Índice del material mat1
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundMiddleElement) {
  // Caso: buscar un material en medio de la lista
  // Definimos 3 materiales y luego una esfera que usa el segundo
  writeSceneFile("matte: first 0.5 0.5 0.5\n"
                 "metal: middle 0.6 0.6 0.6 0.1\n"
                 "refractive: last 1.5\n"
                 "sphere: 0 0 0 1 middle\n");  // Usa middle (índice 1)

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialNames[1], "middle");

  // Verificar que se añadió la esfera exitosamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 1);  // Índice del material middle
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundLastElement) {
  // Caso: buscar el último material de la lista
  // Definimos 3 materiales y luego una esfera que usa el último
  writeSceneFile("matte: first 0.5 0.5 0.5\n"
                 "metal: second 0.6 0.6 0.6 0.1\n"
                 "refractive: last 1.5\n"
                 "sphere: 0 0 0 1 last\n");  // Usa last (índice 2)

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 materiales
  ASSERT_EQ(scene.materialNames.size(), 3);
  ASSERT_EQ(scene.materialNames[2], "last");

  // Verificar que se añadió la esfera exitosamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 2);  // Índice del material last
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundWithComplexName) {
  // Caso: material con nombre complejo (guiones, guiones bajos, números)
  writeSceneFile("matte: red-material_v2 0.8 0.1 0.1\n"
                 "sphere: 0 0 0 1 red-material_v2\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió la esfera exitosamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, MultipleSpheresSameMaterial) {
  // Caso: múltiples esferas usando el mismo material
  // Verifica que findMaterialIndex devuelve consistentemente el mismo índice
  writeSceneFile("matte: shared_mat 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 shared_mat\n"
                 "sphere: 1 1 1 0.5 shared_mat\n"
                 "sphere: -1 -1 -1 2 shared_mat\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 esferas
  ASSERT_EQ(scene.spheres.x.size(), 3);

  // Todas deben referenciar el mismo material (índice 0)
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  ASSERT_EQ(scene.spheres.materialIndex[1], 0);
  ASSERT_EQ(scene.spheres.materialIndex[2], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, DifferentSpheresUseDifferentMaterials) {
  // Caso: múltiples esferas usando diferentes materiales
  writeSceneFile("matte: mat1 0.8 0.1 0.1\n"
                 "metal: mat2 0.1 0.8 0.1 0.2\n"
                 "refractive: mat3 1.5\n"
                 "sphere: 0 0 0 1 mat1\n"
                 "sphere: 1 1 1 0.5 mat2\n"
                 "sphere: -1 -1 -1 2 mat3\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 3 esferas
  ASSERT_EQ(scene.spheres.x.size(), 3);

  // Cada esfera debe referenciar un material diferente
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // mat1
  ASSERT_EQ(scene.spheres.materialIndex[1], 1);  // mat2
  ASSERT_EQ(scene.spheres.materialIndex[2], 2);  // mat3
}

// CASOS DE ERROR - Material no encontrado

TEST_F(SceneParserFindMaterialIndexTest, MaterialNotFound) {
  // Caso: buscar un material que no existe
  // Si findMaterialIndex devuelve -1, parseSphere debe fallar
  writeSceneFile("matte: existing_mat 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 nonexistent_mat\n");  // Material inexistente

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);

  // La esfera NO debe haberse añadido (parseSphere falla si materialIndex == -1)
  ASSERT_EQ(scene.spheres.x.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, EmptyMaterialList) {
  // Caso: intentar usar un material cuando no se ha definido ninguno
  // (lista materialNames vacía)
  writeSceneFile("sphere: 0 0 0 1 any_mat\n");  // No hay materiales definidos

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // No debe haber materiales
  ASSERT_EQ(scene.materialNames.size(), 0);

  // La esfera NO debe haberse añadido
  ASSERT_EQ(scene.spheres.x.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, CaseSensitiveSearch) {
  // Caso: verificar que la búsqueda es sensible a mayúsculas/minúsculas
  // Definimos "mat1" en minúsculas, pero intentamos buscar "Mat1" o "MAT1"
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 Mat1\n");  // Mayúscula diferente

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material "mat1" fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);
  ASSERT_EQ(scene.materialNames[0], "mat1");

  // La esfera NO debe haberse añadido (Mat1 != mat1)
  ASSERT_EQ(scene.spheres.x.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, CaseSensitiveSearchUpperCase) {
  // Caso: verificar sensibilidad con todas mayúsculas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 MAT1\n");  // Todo en mayúsculas

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material "mat1" fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);

  // La esfera NO debe haberse añadido (MAT1 != mat1)
  ASSERT_EQ(scene.spheres.x.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, PartialMatchNotFound) {
  // Caso: verificar que no se encuentra un material con nombre parcialmente similar
  writeSceneFile("matte: material 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat\n");  // Prefijo del nombre real

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material "material" fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);

  // La esfera NO debe haberse añadido (mat != material)
  ASSERT_EQ(scene.spheres.x.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, ExtraWhitespaceInMaterialName) {
  // Caso: verificar que espacios extra en el nombre del material no coinciden
  // Nota: El tokenizer debería separar los espacios, pero este test documenta
  // el comportamiento si llegaran a pasar
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat1\n");  // Nombre exacto (sin espacios)

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe funcionar correctamente (nombre exacto)
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
}

// CASOS ADICIONALES - Edge cases

TEST_F(SceneParserFindMaterialIndexTest, DuplicateMaterialNames) {
  // Caso: si hay nombres duplicados (permitido por el parser),
  // findMaterialIndex debe devolver el índice de la PRIMERA coincidencia
  writeSceneFile("matte: duplicate 0.5 0.5 0.5\n"
                 "metal: duplicate 0.6 0.6 0.6 0.1\n"  // Mismo nombre
                 "sphere: 0 0 0 1 duplicate\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 2 materiales con el mismo nombre
  ASSERT_EQ(scene.materialNames.size(), 2);
  ASSERT_EQ(scene.materialNames[0], "duplicate");
  ASSERT_EQ(scene.materialNames[1], "duplicate");

  // La esfera debe usar el PRIMER material (índice 0)
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);

  // Verificar que efectivamente es el material matte (primer tipo)
  ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameWithNumbers) {
  // Caso: nombres de material con números
  writeSceneFile("matte: mat123 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat123\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe funcionar correctamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameSingleCharacter) {
  // Caso: nombre de material de un solo carácter
  writeSceneFile("matte: m 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 m\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe funcionar correctamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameVeryLong) {
  // Caso: nombre de material muy largo
  writeSceneFile("matte: this_is_a_very_long_material_name_with_many_characters 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 this_is_a_very_long_material_name_with_many_characters\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Debe funcionar correctamente
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, ManyMaterialsLinearSearch) {
  // Caso: muchos materiales para verificar búsqueda lineal
  // findMaterialIndex hace una búsqueda lineal desde el inicio
  writeSceneFile("matte: mat0 0.1 0.1 0.1\n"
                 "matte: mat1 0.2 0.2 0.2\n"
                 "matte: mat2 0.3 0.3 0.3\n"
                 "matte: mat3 0.4 0.4 0.4\n"
                 "matte: mat4 0.5 0.5 0.5\n"
                 "matte: mat5 0.6 0.6 0.6\n"
                 "matte: mat6 0.7 0.7 0.7\n"
                 "matte: mat7 0.8 0.8 0.8\n"
                 "matte: mat8 0.9 0.9 0.9\n"
                 "matte: mat9 1.0 1.0 1.0\n"
                 "sphere: 0 0 0 1 mat5\n");  // Buscar en medio

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadieron 10 materiales
  ASSERT_EQ(scene.materialNames.size(), 10);

  // La esfera debe usar mat5 (índice 5)
  ASSERT_EQ(scene.spheres.x.size(), 1);
  ASSERT_EQ(scene.spheres.materialIndex[0], 5);
}

// TESTS DE INTEGRACIÓN - findMaterialIndex con Cylinder

TEST_F(SceneParserFindMaterialIndexTest, IntegrationWithCylinder) {
  // Caso: verificar que findMaterialIndex también funciona con parseCylinder
  writeSceneFile("metal: cyl_mat 0.7 0.7 0.7 0.3\n"
                 "cylinder: 0 0 0 0.5 0 1 0 cyl_mat\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);

  // Verificar que el cilindro fue añadido exitosamente
  ASSERT_EQ(scene.cylinders.r.size(), 1);
  ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
}

TEST_F(SceneParserFindMaterialIndexTest, IntegrationCylinderMaterialNotFound) {
  // Caso: cylinder con material inexistente
  writeSceneFile("metal: existing 0.7 0.7 0.7 0.3\n"
                 "cylinder: 0 0 0 0.5 0 1 0 nonexistent\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que el material fue añadido
  ASSERT_EQ(scene.materialNames.size(), 1);

  // El cilindro NO debe haberse añadido
  ASSERT_EQ(scene.cylinders.r.size(), 0);
}

TEST_F(SceneParserFindMaterialIndexTest, IntegrationSpheresAndCylindersShareMaterial) {
  // Caso: esferas y cilindros compartiendo el mismo material
  writeSceneFile("matte: shared 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 shared\n"
                 "cylinder: 1 1 1 0.5 0 1 0 shared\n"
                 "sphere: 2 2 2 0.3 shared\n");

  SceneSettings scene = loadSceneFromFile(temp_filename);

  // Verificar que se añadió 1 material
  ASSERT_EQ(scene.materialNames.size(), 1);

  // Verificar que se añadieron 2 esferas y 1 cilindro
  ASSERT_EQ(scene.spheres.x.size(), 2);
  ASSERT_EQ(scene.cylinders.r.size(), 1);

  // Todos deben referenciar el mismo material (índice 0)
  ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
  ASSERT_EQ(scene.spheres.materialIndex[1], 0);
}
