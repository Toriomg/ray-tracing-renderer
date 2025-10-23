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
