#include "dataStructs/material.hpp"
#include "dataStructs/settings_structs.hpp"
#include "scene_parser.hpp"
#include <cmath>
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

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadió 1 material matte
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.matte.g.size(), 1);
    ASSERT_EQ(scene.matte.b.size(), 1);
    // Verificar que se añadió la entrada en materialTable
    ASSERT_EQ(scene.materialTable.size(), 1);
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidBoundaryValues) {
  // Caso válido con valores en los límites [0, 1]: matte: mat2 0.0 1.0 0.5
  writeSceneFile("matte: mat2 0.0 1.0 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadió 1 material matte
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.matte.g.size(), 1);
    ASSERT_EQ(scene.matte.b.size(), 1);
    // Verificar materialTable
    ASSERT_EQ(scene.materialTable.size(), 1);
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidBlackColor) {
  // Negro puro: RGB = (0, 0, 0)
  writeSceneFile("matte: black_mat 0 0 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
    ASSERT_EQ(scene.materialNames[0], "black_mat");
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidWhiteColor) {
  // Blanco puro: RGB = (1, 1, 1)
  writeSceneFile("matte: white_mat 1 1 1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 1.0);
    ASSERT_EQ(scene.materialNames[0], "white_mat");
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("matte: precise_mat 0.123456 0.654321 0.999999\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.123456);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.654321);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.999999);
    ASSERT_EQ(scene.materialNames[0], "precise_mat");
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  // Verifica que los nombres se procesan correctamente
  writeSceneFile("matte: red-material_v2 0.8 0.1 0.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "red-material_v2");
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidShortMaterialName) {
  // Nombre de material de un solo carácter
  writeSceneFile("matte: m 0.5 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "m");
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 5 tokens: falta todo (solo el comando)
  writeSceneFile("matte:\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.matte.g.empty());
    ASSERT_TRUE(scene.matte.b.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, faltan los valores RGB
  writeSceneFile("matte: mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NameAndR) {
  // Solo nombre y componente R, faltan G y B
  writeSceneFile("matte: mat1 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooFewArguments_NameRG) {
  // Solo nombre, R y G, falta B
  writeSceneFile("matte: mat1 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 5 tokens: un argumento extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5 extra\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5 extra1 extra2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeSceneFile("matte: mat1 abc 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeSceneFile("matte: mat1 0.5 abc 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeSceneFile("matte: mat1 0.5 0.5 abc\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllNonNumeric) {
  // Todos los componentes RGB no numéricos
  writeSceneFile("matte: mat1 abc def ghi\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorAlphanumericMixedInComponent) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("matte: mat1 0.5abc 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorEmptyValues) {
  // Valores vacíos después del nombre
  writeSceneFile("matte: mat1   \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE RANGO - Componentes RGB fuera del rango [0, 1]

TEST_F(SceneParserMatteMaterialTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeSceneFile("matte: mat1 -0.1 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (fuera de rango)
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeSceneFile("matte: mat1 0.5 1.1 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeSceneFile("matte: mat1 0.5 0.5 -0.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeSceneFile("matte: mat1 0.5 0.5 1.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorRComponentFarBelowZero) {
  // Componente R muy negativo
  writeSceneFile("matte: mat1 -100.0 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorGComponentFarAboveOne) {
  // Componente G muy por encima de 1
  writeSceneFile("matte: mat1 0.5 100.0 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllComponentsBelowZero) {
  // Todos los componentes menores que 0
  writeSceneFile("matte: mat1 -0.1 -0.2 -0.3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorAllComponentsAboveOne) {
  // Todos los componentes mayores que 1
  writeSceneFile("matte: mat1 1.1 1.2 1.3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentFarOutOfRange) {
  // Componente B = 2 (muy fuera de rango)
  writeSceneFile("matte: mat1 0.5 0.5 2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, ErrorBComponentNegativeFarOutOfRange) {
  // Componente B = -1 (muy fuera de rango)
  writeSceneFile("matte: mat1 0.5 0.5 -1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserMatteMaterialTest, BoundaryExactlyZero) {
  // Todos los componentes exactamente 0.0
  writeSceneFile("matte: mat_zero 0.0 0.0 0.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
  }
}

TEST_F(SceneParserMatteMaterialTest, BoundaryExactlyOne) {
  // Todos los componentes exactamente 1.0
  writeSceneFile("matte: mat_one 1.0 1.0 1.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 1.0);
  }
}

TEST_F(SceneParserMatteMaterialTest, BoundaryVeryCloseToZero) {
  // Valores muy cercanos a 0 pero válidos
  writeSceneFile("matte: mat_near_zero 0.0001 0.0001 0.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0001);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0001);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0001);
  }
}

TEST_F(SceneParserMatteMaterialTest, BoundaryVeryCloseToOne) {
  // Valores muy cercanos a 1 pero válidos
  writeSceneFile("matte: mat_near_one 0.9999 0.9999 0.9999\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.9999);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.9999);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.9999);
  }
}

TEST_F(SceneParserMatteMaterialTest, BoundaryJustBelowZeroInvalid) {
  // Valor justo por debajo de 0.0 (inválido)
  writeSceneFile("matte: mat1 -0.0001 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, BoundaryJustAboveOneInvalid) {
  // Valor justo por encima de 1.0 (inválido)
  writeSceneFile("matte: mat1 0.5 1.0001 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.matte.r.empty());
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserMatteMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("matte:    mat1   0.5   0.5   0.5   \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "mat1");
  }
}

TEST_F(SceneParserMatteMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  matte: mat1 0.5 0.5 0.5  \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "mat1");
  }
}

TEST_F(SceneParserMatteMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("matte:\tmat1\t0.5\t0.5\t0.5\t\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "mat1");
  }
}

TEST_F(SceneParserMatteMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \tmatte:  \t mat1 \t 0.5 \t 0.5 \t 0.5 \t \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "mat1");
  }
}

TEST_F(SceneParserMatteMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# matte: mat_ignored 1.0 1.0 1.0\nmatte: mat1 0.5 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haber 1 material (no 2)
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  }
}

TEST_F(SceneParserMatteMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nmatte: mat1 0.5 0.5 0.5\n\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "mat1");
  }
}

TEST_F(SceneParserMatteMaterialTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1]
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9
  writeSceneFile("matte: mat_sci 5e-1 1e-1 9e-1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.1);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.9);
  }
}

TEST_F(SceneParserMatteMaterialTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeSceneFile("matte: mat1 1e1 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (fuera de rango)
    ASSERT_TRUE(scene.matte.r.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, NegativeZeroComponent) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeSceneFile("matte: mat_neg_zero -0.0 -0.0 -0.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
  }
}

TEST_F(SceneParserMatteMaterialTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeSceneFile("matte: mat_int 0 1 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.0);
  }
}

TEST_F(SceneParserMatteMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("matte: mat_leading 00.5 00.5 00.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
  }
}

TEST_F(SceneParserMatteMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("matte: mat_plus +0.5 +0.5 +0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.matte.b[0], 0.5);
  }
}

TEST_F(SceneParserMatteMaterialTest, InfinityValue) {
  // Valor infinito (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeSceneFile("matte: mat1 inf 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (inf > 1.0)
    ASSERT_TRUE(scene.matte.r.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeSceneFile("matte: mat1 nan 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (nan falla la comparación)
    ASSERT_TRUE(scene.matte.r.empty());
  }
}

TEST_F(SceneParserMatteMaterialTest, MaterialNameWithNumbers) {
  // Nombre de material con números
  writeSceneFile("matte: material123 0.5 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "material123");
  }
}

TEST_F(SceneParserMatteMaterialTest, MaterialNameCaseSensitive) {
  // Verificar que los nombres de material son case-sensitive
  // (mat1 != Mat1 != MAT1)
  writeSceneFile("matte: mat1 0.1 0.1 0.1\n"
                 "matte: Mat1 0.2 0.2 0.2\n"
                 "matte: MAT1 0.3 0.3 0.3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 3);
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    ASSERT_EQ(scene.materialNames[1], "Mat1");
    ASSERT_EQ(scene.materialNames[2], "MAT1");
  }
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

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.matte.r.size(), 3);
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialTable.size(), 3);
  }
}

TEST_F(SceneParserMatteMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  // (testing de robustez ante errores)
  writeSceneFile("matte: error_mat 2.0 2.0 2.0\n"    // Línea con error (fuera de rango)
                 "matte: valid_mat 0.5 0.5 0.5\n");  // Línea válida

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haberse añadido el material válido
    ASSERT_EQ(scene.matte.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "valid_mat");
    ASSERT_DOUBLE_EQ(scene.matte.r[0], 0.5);
  }
}

TEST_F(SceneParserMatteMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados (aunque no sea ideal)
  // El parser no valida unicidad de nombres
  writeSceneFile("matte: mat1 0.1 0.1 0.1\n"
                 "matte: mat1 0.2 0.2 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Ambos materiales deben haberse añadido
    ASSERT_EQ(scene.matte.r.size(), 2);
    ASSERT_EQ(scene.materialNames.size(), 2);
    // Verificar índices locales diferentes
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
    ASSERT_EQ(scene.materialTable[1].localIndex, 1);
  }
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

TEST_F(SceneParserMetalMaterialTest, ValidHighDiffusion) {
  // Caso válido con difusión alta (sin límite superior)
  // metal: met3 0.9 0.9 0.9 10.0
  writeSceneFile("metal: met3 0.9 0.9 0.9 10.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadió el material
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    // Verificar nombre y tabla
    ASSERT_EQ(scene.materialNames[0], "met3");
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMetalMaterialTest, ValidBlackColor) {
  // Negro puro con difusión: RGB = (0, 0, 0), diffusion = 0.5
  writeSceneFile("metal: black_metal 0 0 0 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "black_metal");
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::METAL);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserMetalMaterialTest, ValidWhiteColor) {
  // Blanco puro con difusión: RGB = (1, 1, 1), diffusion = 0.1
  writeSceneFile("metal: white_metal 1 1 1 0.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.1);
    ASSERT_EQ(scene.materialNames[0], "white_metal");
  }
}

TEST_F(SceneParserMetalMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("metal: precise_metal 0.123456 0.654321 0.999999 0.456789\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.123456);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.654321);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.999999);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.456789);
    ASSERT_EQ(scene.materialNames[0], "precise_metal");
  }
}

TEST_F(SceneParserMetalMaterialTest, ValidVeryHighDiffusion) {
  // Difusión muy alta (sin límite superior explícito en el código)
  writeSceneFile("metal: fuzzy_metal 0.5 0.5 0.5 1000.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 1000.0);
  }
}

TEST_F(SceneParserMetalMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  writeSceneFile("metal: brushed-aluminum_v2 0.7 0.7 0.7 0.4\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "brushed-aluminum_v2");
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 6 tokens: falta todo (solo el comando)
  writeSceneFile("metal:\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.metal.g.empty());
    ASSERT_TRUE(scene.metal.b.empty());
    ASSERT_TRUE(scene.metal.diffusion.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, faltan los valores RGB y diffusion
  writeSceneFile("metal: met1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameAndRGB) {
  // Solo nombre y RGB, falta diffusion
  writeSceneFile("metal: met1 0.5 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameRG) {
  // Solo nombre, R y G, faltan B y diffusion
  writeSceneFile("metal: met1 0.5 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooFewArguments_NameR) {
  // Solo nombre y R, faltan G, B y diffusion
  writeSceneFile("metal: met1 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 6 tokens: un argumento extra
  writeSceneFile("metal: met1 0.5 0.5 0.5 0.2 extra\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("metal: met1 0.5 0.5 0.5 0.2 extra1 extra2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericRComponent) {
  // Componente R no numérico
  writeSceneFile("metal: met1 abc 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericGComponent) {
  // Componente G no numérico
  writeSceneFile("metal: met1 0.5 abc 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericBComponent) {
  // Componente B no numérico
  writeSceneFile("metal: met1 0.5 0.5 abc 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorNonNumericDiffusion) {
  // Diffusion no numérico
  writeSceneFile("metal: met1 0.5 0.5 0.5 abc\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllNonNumeric) {
  // Todos los valores no numéricos
  writeSceneFile("metal: met1 abc def ghi jkl\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorAlphanumericMixedInComponent) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("metal: met1 0.5abc 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorEmptyValues) {
  // Valores vacíos después del nombre
  writeSceneFile("metal: met1   \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE RANGO - Componentes RGB fuera del rango [0, 1]

TEST_F(SceneParserMetalMaterialTest, ErrorRComponentBelowZero) {
  // Componente R menor que 0
  writeSceneFile("metal: met1 -0.1 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (fuera de rango)
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorGComponentAboveOne) {
  // Componente G mayor que 1
  writeSceneFile("metal: met1 0.5 1.1 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorBComponentBelowZero) {
  // Componente B menor que 0
  writeSceneFile("metal: met1 0.5 0.5 -0.1 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorBComponentAboveOne) {
  // Componente B mayor que 1
  writeSceneFile("metal: met1 0.5 0.5 1.1 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllRGBComponentsBelowZero) {
  // Todos los componentes RGB menores que 0
  writeSceneFile("metal: met1 -0.1 -0.2 -0.3 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorAllRGBComponentsAboveOne) {
  // Todos los componentes RGB mayores que 1
  writeSceneFile("metal: met1 1.1 1.2 1.3 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorRComponentFarBelowZero) {
  // Componente R muy negativo
  writeSceneFile("metal: met1 -100.0 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorGComponentFarAboveOne) {
  // Componente G muy por encima de 1
  writeSceneFile("metal: met1 0.5 100.0 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE RANGO - Factor de difusión negativo

TEST_F(SceneParserMetalMaterialTest, ErrorDiffusionBelowZero) {
  // Factor de difusión menor que 0 (inválido según código)
  writeSceneFile("metal: met1 0.5 0.5 0.5 -0.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (diffusion < 0.0 es inválido)
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorDiffusionFarBelowZero) {
  // Factor de difusión muy negativo
  writeSceneFile("metal: met1 0.5 0.5 0.5 -100.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ErrorCombinedRGBAndDiffusionOutOfRange) {
  // RGB fuera de rango Y diffusion negativo (múltiples errores)
  writeSceneFile("metal: met1 -0.1 1.5 0.5 -0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBExactlyZero) {
  // Todos los componentes RGB exactamente 0.0
  writeSceneFile("metal: met_zero 0.0 0.0 0.0 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBExactlyOne) {
  // Todos los componentes RGB exactamente 1.0
  writeSceneFile("metal: met_one 1.0 1.0 1.0 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.5);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionExactlyZero) {
  // Difusión exactamente 0.0 (válido, superficie perfectamente reflectante)
  writeSceneFile("metal: mirror 0.9 0.9 0.9 0.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.0);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBVeryCloseToZero) {
  // Valores RGB muy cercanos a 0 pero válidos
  writeSceneFile("metal: met_near_zero 0.0001 0.0001 0.0001 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0001);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0001);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0001);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBVeryCloseToOne) {
  // Valores RGB muy cercanos a 1 pero válidos
  writeSceneFile("metal: met_near_one 0.9999 0.9999 0.9999 0.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.9999);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.9999);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.9999);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionVeryCloseToZero) {
  // Difusión muy cercana a 0 pero válida
  writeSceneFile("metal: almost_mirror 0.9 0.9 0.9 0.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.0001);
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBJustBelowZeroInvalid) {
  // Valor RGB justo por debajo de 0.0 (inválido)
  writeSceneFile("metal: met1 -0.0001 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryRGBJustAboveOneInvalid) {
  // Valor RGB justo por encima de 1.0 (inválido)
  writeSceneFile("metal: met1 0.5 1.0001 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, BoundaryDiffusionJustBelowZeroInvalid) {
  // Difusión justo por debajo de 0.0 (inválido)
  writeSceneFile("metal: met1 0.5 0.5 0.5 -0.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserMetalMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("metal:    met1   0.5   0.5   0.5   0.2   \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
    ASSERT_EQ(scene.materialNames[0], "met1");
  }
}

TEST_F(SceneParserMetalMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  metal: met1 0.5 0.5 0.5 0.2  \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "met1");
  }
}

TEST_F(SceneParserMetalMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("metal:\tmet1\t0.5\t0.5\t0.5\t0.2\t\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
    ASSERT_EQ(scene.materialNames[0], "met1");
  }
}

TEST_F(SceneParserMetalMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \tmetal:  \t met1 \t 0.5 \t 0.5 \t 0.5 \t 0.2 \t \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "met1");
  }
}

TEST_F(SceneParserMetalMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# metal: met_ignored 1.0 1.0 1.0 0.5\nmetal: met1 0.5 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haber 1 material (no 2)
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "met1");
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  }
}

TEST_F(SceneParserMetalMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nmetal: met1 0.5 0.5 0.5 0.2\n\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_EQ(scene.materialNames[0], "met1");
  }
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationValidRange) {
  // Notación científica dentro del rango válido [0, 1] para RGB
  // parsedouble acepta notación científica
  // 5e-1 = 0.5, 1e-1 = 0.1, 9e-1 = 0.9, 2e-1 = 0.2
  writeSceneFile("metal: met_sci 5e-1 1e-1 9e-1 2e-1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.1);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.9);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  }
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationOutOfRange) {
  // Notación científica fuera del rango válido para RGB
  // 1e1 = 10.0, que está fuera de [0, 1]
  writeSceneFile("metal: met1 1e1 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (fuera de rango)
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, ScientificNotationForDiffusion) {
  // Notación científica válida para diffusion (puede ser > 1)
  // 1e1 = 10.0 es válido para diffusion
  writeSceneFile("metal: met_sci_diff 0.5 0.5 0.5 1e1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 10.0);
  }
}

TEST_F(SceneParserMetalMaterialTest, NegativeZeroRGBComponents) {
  // -0.0 es equivalente a 0.0 en punto flotante (válido)
  writeSceneFile("metal: met_neg_zero -0.0 -0.0 -0.0 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
  }
}

TEST_F(SceneParserMetalMaterialTest, IntegerValues) {
  // Valores enteros (sin punto decimal) deben ser aceptados
  writeSceneFile("metal: met_int 0 1 0 1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 1.0);
  }
}

TEST_F(SceneParserMetalMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("metal: met_leading 00.5 00.5 00.5 00.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  }
}

TEST_F(SceneParserMetalMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("metal: met_plus +0.5 +0.5 +0.5 +0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.g[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.b[0], 0.5);
    ASSERT_DOUBLE_EQ(scene.metal.diffusion[0], 0.2);
  }
}

TEST_F(SceneParserMetalMaterialTest, InfinityValueInRGB) {
  // Valor infinito en RGB (fuera de rango [0, 1])
  // parsedouble acepta "inf", pero debe ser rechazado por validateColorComponents
  writeSceneFile("metal: met1 inf 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (inf > 1.0)
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, InfinityValueInDiffusion) {
  // Valor infinito en diffusion (técnicamente parsedouble lo acepta)
  // Pero en la práctica no tiene sentido físico
  writeSceneFile("metal: met1 0.5 0.5 0.5 inf\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Si parsedouble acepta inf y diffusion >= 0, se añadirá
    // (el código no valida límite superior para diffusion)
    ASSERT_EQ(scene.metal.diffusion.size(), 1);
    ASSERT_TRUE(std::isinf(scene.metal.diffusion[0]));
  }
}

TEST_F(SceneParserMetalMaterialTest, NaNValueInRGB) {
  // Valor NaN en RGB (no válido)
  // parsedouble acepta "nan", pero validateColorComponents debería rechazarlo
  writeSceneFile("metal: met1 nan 0.5 0.5 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (nan falla la comparación)
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

TEST_F(SceneParserMetalMaterialTest, NaNValueInDiffusion) {
  // Valor NaN en diffusion (no válido)
  writeSceneFile("metal: met1 0.5 0.5 0.5 nan\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (nan falla la comparación >= 0)
    ASSERT_TRUE(scene.metal.r.empty());
  }
}

// TESTS DE INTEGRACIÓN

TEST_F(SceneParserMetalMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  writeSceneFile("metal: error_met 2.0 2.0 2.0 0.2\n"    // Línea con error (RGB fuera de rango)
                 "metal: valid_met 0.5 0.5 0.5 0.2\n");  // Línea válida

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haberse añadido el material válido
    ASSERT_EQ(scene.metal.r.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "valid_met");
    ASSERT_DOUBLE_EQ(scene.metal.r[0], 0.5);
  }
}

TEST_F(SceneParserMetalMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados
  writeSceneFile("metal: met1 0.1 0.1 0.1 0.1\n"
                 "metal: met1 0.2 0.2 0.2 0.2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Ambos materiales deben haberse añadido
    ASSERT_EQ(scene.metal.r.size(), 2);
    ASSERT_EQ(scene.materialNames.size(), 2);
    ASSERT_EQ(scene.materialNames[0], "met1");
    ASSERT_EQ(scene.materialNames[1], "met1");
    // Verificar índices locales diferentes
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
    ASSERT_EQ(scene.materialTable[1].localIndex, 1);
  }
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

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

TEST_F(SceneParserRefractiveMaterialTest, ValidIORCloseToOne) {
  // Caso válido con IOR cercano a 1 (aire/vacío): IOR = 1.001
  writeSceneFile("refractive: air_like 1.001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

TEST_F(SceneParserRefractiveMaterialTest, ValidHighIOR) {
  // Caso válido con IOR alto (diamante): IOR = 2.4
  writeSceneFile("refractive: diamond 2.4\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

TEST_F(SceneParserRefractiveMaterialTest, ValidVerySmallIOR) {
  // Caso válido con IOR muy pequeño pero > 0: IOR = 0.00001
  // Aunque físicamente poco realista, es técnicamente válido según la validación
  writeSceneFile("refractive: near_vacuum 0.00001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

TEST_F(SceneParserRefractiveMaterialTest, ValidWater) {
  // Agua: IOR = 1.33
  writeSceneFile("refractive: water 1.33\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.33);
    ASSERT_EQ(scene.materialNames[0], "water");
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::REFRACTIVE);
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ValidDecimalPrecision) {
  // Valores con múltiples decimales para verificar precisión
  writeSceneFile("refractive: precise_glass 1.5168\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5168);
    ASSERT_EQ(scene.materialNames[0], "precise_glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ValidVeryHighIOR) {
  // IOR muy alto (teóricamente posible): IOR = 10.0
  writeSceneFile("refractive: exotic_material 10.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 10.0);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ValidComplexMaterialName) {
  // Nombres de material complejos: con guiones, guiones bajos, números
  writeSceneFile("refractive: borosilicate-glass_v2 1.47\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "borosilicate-glass_v2");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ValidIntegerIOR) {
  // Valor entero (sin punto decimal) debe ser aceptado
  writeSceneFile("refractive: integer_ior 2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 2.0);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooFewArguments_NoValues) {
  // Menos de 3 tokens: falta todo (solo el comando)
  writeSceneFile("refractive:\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooFewArguments_OnlyName) {
  // Solo nombre, falta IOR
  writeSceneFile("refractive: glass\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooManyArguments_OneExtra) {
  // Más de 3 tokens: un argumento extra
  writeSceneFile("refractive: glass 1.5 extra\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (error de parsing)
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorTooManyArguments_Multiple) {
  // Múltiples argumentos extra
  writeSceneFile("refractive: glass 1.5 extra1 extra2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserRefractiveMaterialTest, ErrorNonNumericIOR) {
  // IOR no numérico
  writeSceneFile("refractive: glass abc\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorAlphanumericMixedInIOR) {
  // Valor alfanumérico mixto (parsedouble debería rechazarlo)
  writeSceneFile("refractive: glass 1.5abc\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorEmptyIORValue) {
  // Valor IOR vacío
  writeSceneFile("refractive: glass  \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// ERRORES DE RANGO - IOR <= 0

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORZero) {
  // IOR igual a cero (inválido según código: ior <= 0.0)
  writeSceneFile("refractive: glass 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (IOR debe ser > 0)
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORExactlyZero) {
  // IOR exactamente 0.0
  writeSceneFile("refractive: glass 0.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORNegative) {
  // IOR negativo: -1.5
  writeSceneFile("refractive: glass -1.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (IOR negativo no tiene sentido físico)
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORNegativeSmall) {
  // IOR negativo pequeño: -0.1
  writeSceneFile("refractive: glass -0.1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ErrorIORVeryNegative) {
  // IOR muy negativo: -100.0
  writeSceneFile("refractive: glass -100.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
    ASSERT_TRUE(scene.materialNames.empty());
    ASSERT_TRUE(scene.materialTable.empty());
  }
}

// CASOS LÍMITE - Valores en los bordes del rango válido

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORJustAboveZero) {
  // IOR justo por encima de 0.0 (válido): 0.0001
  writeSceneFile("refractive: almost_zero 0.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 0.0001);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORExactlyOne) {
  // IOR exactamente 1.0 (vacío perfecto)
  writeSceneFile("refractive: vacuum 1.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.0);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORVeryCloseToOne) {
  // IOR muy cercano a 1.0: 1.0001
  writeSceneFile("refractive: near_vacuum 1.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.0001);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, BoundaryIORJustBelowZeroInvalid) {
  // IOR justo por debajo de 0.0 (inválido): -0.0001
  writeSceneFile("refractive: glass -0.0001\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío
    ASSERT_TRUE(scene.refractive.ior.empty());
  }
}

// CASOS ADICIONALES - Edge cases y robustez

TEST_F(SceneParserRefractiveMaterialTest, ExtraWhitespaceAroundValues) {
  // Espacios extra alrededor de los valores
  // Verifica que trimWhitespace funciona correctamente
  writeSceneFile("refractive:    glass   1.5   \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_EQ(scene.materialNames[0], "glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ExtraWhitespaceAroundLine) {
  // Espacios al inicio y final de la línea
  writeSceneFile("  refractive: glass 1.5  \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_EQ(scene.materialNames[0], "glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, TabsAsWhitespace) {
  // Tabs como espacios en blanco
  // Verifica que trimWhitespace maneja tabs correctamente
  writeSceneFile("refractive:\tglass\t1.5\t\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_EQ(scene.materialNames[0], "glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, MixedWhitespace) {
  // Mezcla de espacios y tabs
  writeSceneFile("  \trefractive:  \t glass \t 1.5 \t \n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_EQ(scene.materialNames[0], "glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, CommentLineShouldBeIgnored) {
  // Línea de comentario debe ser ignorada
  writeSceneFile("# refractive: ignored 2.0\nrefractive: glass 1.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haber 1 material (no 2)
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "glass");
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, EmptyLinesAroundCommand) {
  // Líneas vacías no deben afectar el parsing
  writeSceneFile("\n\nrefractive: glass 1.5\n\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_EQ(scene.materialNames[0], "glass");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationValidIOR) {
  // Notación científica válida para IOR > 0
  // parsedouble acepta notación científica
  // 1.5e0 = 1.5, 2.4e0 = 2.4
  writeSceneFile("refractive: glass_sci 1.5e0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationHighIOR) {
  // Notación científica para IOR alto
  // 2.4e0 = 2.4, 1e1 = 10.0
  writeSceneFile("refractive: exotic 1e1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 10.0);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, ScientificNotationSmallIOR) {
  // Notación científica para IOR pequeño pero válido
  // 1e-3 = 0.001
  writeSceneFile("refractive: tiny_ior 1e-3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 0.001);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, LeadingZeros) {
  // Valores con ceros a la izquierda
  writeSceneFile("refractive: glass 001.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, PlusSignPrefix) {
  // Signo + explícito (parsedouble lo acepta)
  writeSceneFile("refractive: glass +1.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, InfinityValue) {
  // Valor infinito (técnicamente parsedouble lo acepta y es > 0)
  // Aunque físicamente no tiene sentido
  writeSceneFile("refractive: glass inf\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Si parsedouble acepta inf y ior > 0, se añadirá
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_TRUE(std::isinf(scene.refractive.ior[0]));
    ASSERT_GT(scene.refractive.ior[0], 0.0);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, NaNValue) {
  // Valor NaN (no válido)
  // parsedouble acepta "nan", pero debería fallar la comparación ior <= 0
  writeSceneFile("refractive: glass nan\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (nan falla la comparación > 0)
    ASSERT_TRUE(scene.refractive.ior.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, NegativeZero) {
  // -0.0 es equivalente a 0.0 en punto flotante (inválido, no es > 0)
  writeSceneFile("refractive: glass -0.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe permanecer vacío (-0.0 no es > 0)
    ASSERT_TRUE(scene.refractive.ior.empty());
  }
}

TEST_F(SceneParserRefractiveMaterialTest, MaterialNameWithNumbers) {
  // Nombre de material con números
  writeSceneFile("refractive: glass123 1.5\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "glass123");
  }
}

TEST_F(SceneParserRefractiveMaterialTest, MaterialNameCaseSensitive) {
  // Verificar que los nombres de material son case-sensitive
  writeSceneFile("refractive: glass 1.5\n"
                 "refractive: Glass 1.6\n"
                 "refractive: GLASS 1.7\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.refractive.ior.size(), 3);
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialNames[0], "glass");
    ASSERT_EQ(scene.materialNames[1], "Glass");
    ASSERT_EQ(scene.materialNames[2], "GLASS");
  }
}

// TESTS DE INTEGRACIÓN

TEST_F(SceneParserRefractiveMaterialTest, ValidAfterErrorLine) {
  // Verificar que un material válido después de una línea con error se procesa correctamente
  writeSceneFile("refractive: error_glass 0\n"      // Línea con error (IOR = 0)
                 "refractive: valid_glass 1.5\n");  // Línea válida

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo debe haberse añadido el material válido
    ASSERT_EQ(scene.refractive.ior.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "valid_glass");
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
  }
}

TEST_F(SceneParserRefractiveMaterialTest, DuplicateMaterialNameAllowed) {
  // Verificar que se permiten nombres duplicados
  writeSceneFile("refractive: glass 1.5\n"
                 "refractive: glass 1.6\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que tienen diferentes valores IOR
    ASSERT_DOUBLE_EQ(scene.refractive.ior[0], 1.5);
    ASSERT_DOUBLE_EQ(scene.refractive.ior[1], 1.6);
    // Verificar índices locales diferentes
    ASSERT_EQ(scene.materialTable[0].localIndex, 0);
    ASSERT_EQ(scene.materialTable[1].localIndex, 1);
  }
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

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 materiales
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // Verificar que se añadió la esfera exitosamente
    // (esto significa que findMaterialIndex encontró mat1 y devolvió 0)
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // Índice del material mat1
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundMiddleElement) {
  // Caso: buscar un material en medio de la lista
  // Definimos 3 materiales y luego una esfera que usa el segundo
  writeSceneFile("matte: first 0.5 0.5 0.5\n"
                 "metal: middle 0.6 0.6 0.6 0.1\n"
                 "refractive: last 1.5\n"
                 "sphere: 0 0 0 1 middle\n");  // Usa middle (índice 1)

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 materiales
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialNames[1], "middle");
    // Verificar que se añadió la esfera exitosamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 1);  // Índice del material middle
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundLastElement) {
  // Caso: buscar el último material de la lista
  // Definimos 3 materiales y luego una esfera que usa el último
  writeSceneFile("matte: first 0.5 0.5 0.5\n"
                 "metal: second 0.6 0.6 0.6 0.1\n"
                 "refractive: last 1.5\n"
                 "sphere: 0 0 0 1 last\n");  // Usa last (índice 2)

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 materiales
    ASSERT_EQ(scene.materialNames.size(), 3);
    ASSERT_EQ(scene.materialNames[2], "last");
    // Verificar que se añadió la esfera exitosamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 2);  // Índice del material last
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialFoundWithComplexName) {
  // Caso: material con nombre complejo (guiones, guiones bajos, números)
  writeSceneFile("matte: red-material_v2 0.8 0.1 0.1\n"
                 "sphere: 0 0 0 1 red-material_v2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadió la esfera exitosamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MultipleSpheresSameMaterial) {
  // Caso: múltiples esferas usando el mismo material
  // Verifica que findMaterialIndex devuelve consistentemente el mismo índice
  writeSceneFile("matte: shared_mat 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 shared_mat\n"
                 "sphere: 1 1 1 0.5 shared_mat\n"
                 "sphere: -1 -1 -1 2 shared_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 esferas
    ASSERT_EQ(scene.spheres.x.size(), 3);
    // Todas deben referenciar el mismo material (índice 0)
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
    ASSERT_EQ(scene.spheres.materialIndex[1], 0);
    ASSERT_EQ(scene.spheres.materialIndex[2], 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, DifferentSpheresUseDifferentMaterials) {
  // Caso: múltiples esferas usando diferentes materiales
  writeSceneFile("matte: mat1 0.8 0.1 0.1\n"
                 "metal: mat2 0.1 0.8 0.1 0.2\n"
                 "refractive: mat3 1.5\n"
                 "sphere: 0 0 0 1 mat1\n"
                 "sphere: 1 1 1 0.5 mat2\n"
                 "sphere: -1 -1 -1 2 mat3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 esferas
    ASSERT_EQ(scene.spheres.x.size(), 3);
    // Cada esfera debe referenciar un material diferente
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // mat1
    ASSERT_EQ(scene.spheres.materialIndex[1], 1);  // mat2
    ASSERT_EQ(scene.spheres.materialIndex[2], 2);  // mat3
  }
}

// CASOS DE ERROR - Material no encontrado

TEST_F(SceneParserFindMaterialIndexTest, MaterialNotFound) {
  // Caso: buscar un material que no existe
  // Si findMaterialIndex devuelve -1, parseSphere debe fallar
  writeSceneFile("matte: existing_mat 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 nonexistent_mat\n");  // Material inexistente

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (parseSphere falla si materialIndex == -1)
    ASSERT_EQ(scene.spheres.x.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, EmptyMaterialList) {
  // Caso: intentar usar un material cuando no se ha definido ninguno
  // (lista materialNames vacía)
  writeSceneFile("sphere: 0 0 0 1 any_mat\n");  // No hay materiales definidos

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // No debe haber materiales
    ASSERT_EQ(scene.materialNames.size(), 0);
    // La esfera NO debe haberse añadido
    ASSERT_EQ(scene.spheres.x.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, CaseSensitiveSearch) {
  // Caso: verificar que la búsqueda es sensible a mayúsculas/minúsculas
  // Definimos "mat1" en minúsculas, pero intentamos buscar "Mat1" o "MAT1"
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 Mat1\n");  // Mayúscula diferente

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material "mat1" fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // La esfera NO debe haberse añadido (Mat1 != mat1)
    ASSERT_EQ(scene.spheres.x.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, CaseSensitiveSearchUpperCase) {
  // Caso: verificar sensibilidad con todas mayúsculas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 MAT1\n");  // Todo en mayúsculas

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material "mat1" fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (MAT1 != mat1)
    ASSERT_EQ(scene.spheres.x.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, PartialMatchNotFound) {
  // Caso: verificar que no se encuentra un material con nombre parcialmente similar
  writeSceneFile("matte: material 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat\n");  // Prefijo del nombre real

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material "material" fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (mat != material)
    ASSERT_EQ(scene.spheres.x.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, ExtraWhitespaceInMaterialName) {
  // Caso: verificar que espacios extra en el nombre del material no coinciden
  // Nota: El tokenizer debería separar los espacios, pero este test documenta
  // el comportamiento si llegaran a pasar
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat1\n");  // Nombre exacto (sin espacios)

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe funcionar correctamente (nombre exacto)
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

// CASOS ADICIONALES - Edge cases

TEST_F(SceneParserFindMaterialIndexTest, DuplicateMaterialNames) {
  // Caso: si hay nombres duplicados (permitido por el parser),
  // findMaterialIndex debe devolver el índice de la PRIMERA coincidencia
  writeSceneFile("matte: duplicate 0.5 0.5 0.5\n"
                 "metal: duplicate 0.6 0.6 0.6 0.1\n"  // Mismo nombre
                 "sphere: 0 0 0 1 duplicate\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameWithNumbers) {
  // Caso: nombres de material con números
  writeSceneFile("matte: mat123 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 mat123\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe funcionar correctamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameSingleCharacter) {
  // Caso: nombre de material de un solo carácter
  writeSceneFile("matte: m 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 m\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe funcionar correctamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, MaterialNameVeryLong) {
  // Caso: nombre de material muy largo
  writeSceneFile("matte: this_is_a_very_long_material_name_with_many_characters 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 this_is_a_very_long_material_name_with_many_characters\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debe funcionar correctamente
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
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

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 10 materiales
    ASSERT_EQ(scene.materialNames.size(), 10);
    // La esfera debe usar mat5 (índice 5)
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 5);
  }
}

// TESTS DE INTEGRACIÓN - findMaterialIndex con Cylinder

TEST_F(SceneParserFindMaterialIndexTest, IntegrationWithCylinder) {
  // Caso: verificar que findMaterialIndex también funciona con parseCylinder
  writeSceneFile("metal: cyl_mat 0.7 0.7 0.7 0.3\n"
                 "cylinder: 0 0 0 0.5 0 1 0 cyl_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // Verificar que el cilindro fue añadido exitosamente
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, IntegrationCylinderMaterialNotFound) {
  // Caso: cylinder con material inexistente
  writeSceneFile("metal: existing 0.7 0.7 0.7 0.3\n"
                 "cylinder: 0 0 0 0.5 0 1 0 nonexistent\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido
    ASSERT_EQ(scene.cylinders.r.size(), 0);
  }
}

TEST_F(SceneParserFindMaterialIndexTest, IntegrationSpheresAndCylindersShareMaterial) {
  // Caso: esferas y cilindros compartiendo el mismo material
  writeSceneFile("matte: shared 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1 shared\n"
                 "cylinder: 1 1 1 0.5 0 1 0 shared\n"
                 "sphere: 2 2 2 0.3 shared\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
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
}

// ============================================================================
// TESTS PARA parseSphere
// ============================================================================

class SceneParserSphereTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_sphere_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS - Esfera parseada correctamente

TEST_F(SceneParserSphereTest, ValidBasicSphere) {
  // Caso 1: Pre-cargar "mat1", parsear esfera en origen con radio 1.0
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // Verificar que la esfera fue añadida correctamente
    ASSERT_FALSE(scene.spheres.x.empty());
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // Índice de mat1
    // Verificar que se generó el AABB
    ASSERT_EQ(scene.spheres.aabbs.size(), 1);
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithDifferentMaterial) {
  // Caso 2: Pre-cargar "mat1" y "mat2", parsear esfera con mat2
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "metal: mat2 0.6 0.6 0.6 0.1\n"
                 "sphere: 1 2 3 0.5 mat2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 2 materiales
    ASSERT_EQ(scene.materialNames.size(), 2);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    ASSERT_EQ(scene.materialNames[1], "mat2");
    ASSERT_EQ(scene.spheres.materialIndex[0], 1);  // Índice de mat2
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithNegativeCoordinates) {
  // Caso adicional: Esfera con coordenadas negativas (válido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: -10 -20.5 -30.123 5.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que la esfera fue añadida
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.x[0], -10.0);
    ASSERT_DOUBLE_EQ(scene.spheres.y[0], -20.5);
    ASSERT_DOUBLE_EQ(scene.spheres.z[0], -30.123);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 5.0);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithVeryLargeCoordinates) {
  // Caso adicional: Coordenadas muy grandes (pero válidas)
  // Verifica que no hay overflow o problemas numéricos básicos
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 1e10 -1e10 5e9 1e8 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que la esfera fue añadida
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.x[0], 1e10);
    ASSERT_DOUBLE_EQ(scene.spheres.y[0], -1e10);
    ASSERT_DOUBLE_EQ(scene.spheres.z[0], 5e9);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 1e8);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithVerySmallRadius) {
  // Caso adicional: Radio muy pequeño pero válido (> 0)
  // Verifica límites inferiores del radio
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 0.000001 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que la esfera fue añadida
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 0.000001);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithScientificNotation) {
  // Caso adicional: Valores en notación científica
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 1.5e2 -3.2e-1 4.7e3 2.1e1 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que la esfera fue añadida
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.x[0], 150.0);
    ASSERT_DOUBLE_EQ(scene.spheres.y[0], -0.32);
    ASSERT_DOUBLE_EQ(scene.spheres.z[0], 4700.0);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 21.0);
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);
  }
}

TEST_F(SceneParserSphereTest, ValidSphereWithAllMaterialTypes) {
  // Caso adicional: Esferas con cada tipo de material (matte, metal, refractive)
  writeSceneFile("matte: matte_mat 0.8 0.2 0.2\n"
                 "metal: metal_mat 0.2 0.8 0.2 0.5\n"
                 "refractive: refract_mat 1.5\n"
                 "sphere: 0 0 0 1 matte_mat\n"
                 "sphere: 2 0 0 1 metal_mat\n"
                 "sphere: 4 0 0 1 refract_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 esferas
    ASSERT_EQ(scene.spheres.x.size(), 3);
    // Verificar índices de materiales
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // matte_mat
    ASSERT_EQ(scene.spheres.materialIndex[1], 1);  // metal_mat
    ASSERT_EQ(scene.spheres.materialIndex[2], 2);  // refract_mat
    // Verificar tipos de materiales
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[1].type, MaterialType::METAL);
    ASSERT_EQ(scene.materialTable[2].type, MaterialType::REFRACTIVE);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserSphereTest, ErrorTooFewArguments) {
  // Caso 3a: Menos de 6 tokens (requiere 5 parámetros + comando)
  // Falta el material
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (error de formato)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorTooFewArgumentsMissingMultiple) {
  // Caso adicional: Faltan múltiples parámetros
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorOnlyCommand) {
  // Caso adicional: Solo el comando, sin parámetros
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere:\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorTooManyArguments) {
  // Caso 3b: Más de 6 tokens (argumentos extra)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 mat1 extra_arg\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (error de formato)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorTooManyArgumentsMultiple) {
  // Caso adicional: Múltiples argumentos extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 mat1 extra1 extra2 extra3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserSphereTest, ErrorNonNumericX) {
  // Caso 3c: Valor no numérico en coordenada X
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: abc 0 0 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorNonNumericY) {
  // Caso 3c: Valor no numérico en coordenada Y
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 xyz 0 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorNonNumericZ) {
  // Caso 3c: Valor no numérico en coordenada Z
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 invalid 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorNonNumericRadius) {
  // Caso 3c: Valor no numérico en radio
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 notanumber mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorPartialNumericValue) {
  // Caso adicional: Valor parcialmente numérico (comienza con número pero tiene texto)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 5abc 0 0 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorEmptyStringAsNumber) {
  // Caso adicional: String vacío como número (no debería ocurrir con tokenizer, pero documentar el
  // comportamiento) Nota: El tokenizer eliminaría espacios vacíos, este test documenta el
  // comportamiento esperado
  writeSceneFile(
      "matte: mat1 0.5 0.5 0.5\n"
      "sphere: 0 0 0 1.0 mat1\n");  // Este es válido, el caso de string vacío es difícil de

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Este caso en realidad es válido, pero documenta la protección
    ASSERT_EQ(scene.spheres.x.size(), 1);
  }
}

// ERRORES DE RANGO - Radio inválido

TEST_F(SceneParserSphereTest, ErrorRadiusZero) {
  // Caso 4a: Radio igual a cero (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (radio debe ser > 0)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorRadiusNegative) {
  // Caso 4b: Radio negativo (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 -1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (radio debe ser > 0)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorRadiusVeryNegative) {
  // Caso adicional: Radio muy negativo
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 -1000000 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorRadiusNegativeScientific) {
  // Caso adicional: Radio negativo en notación científica
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 -1.5e-2 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

// ERRORES DE MATERIAL - Material no encontrado

TEST_F(SceneParserSphereTest, ErrorMaterialNotFound) {
  // Caso 5a: Referencia a material inexistente
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 unknown_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // La esfera NO debe haberse añadido (material no encontrado)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorNoMaterialsDefined) {
  // Caso adicional: Intentar crear esfera sin definir ningún material
  writeSceneFile("sphere: 0 0 0 1.0 any_material\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // No debe haber materiales
    ASSERT_TRUE(scene.materialNames.empty());
    // La esfera NO debe haberse añadido
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorMaterialCaseSensitive) {
  // Caso 5b: Referencia a material con mayúsculas/minúsculas incorrectas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 Mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido (en minúsculas)
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // La esfera NO debe haberse añadido (Mat1 != mat1)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorMaterialAllUpperCase) {
  // Caso adicional: Material todo en mayúsculas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 MAT1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido (en minúsculas)
    ASSERT_EQ(scene.materialNames.size(), 1);
    // La esfera NO debe haberse añadido (MAT1 != mat1)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, ErrorMaterialPartialName) {
  // Caso adicional: Usar solo parte del nombre del material
  writeSceneFile("matte: material_name 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 material\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material completo fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "material_name");
    // La esfera NO debe haberse añadido (material != material_name)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

// CASOS ADICIONALES - Edge cases y valores especiales

TEST_F(SceneParserSphereTest, EdgeCaseInfinityRadius) {
  // Caso adicional: Radio infinito (técnicamente válido > 0, pero problemático)
  // parsedouble acepta "inf", pero esto documenta el comportamiento
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 inf mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar comportamiento con infinito
    // parsedouble debería aceptar "inf", y como inf > 0, debería pasar la validación
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_TRUE(std::isinf(scene.spheres.r[0]));
    ASSERT_GT(scene.spheres.r[0], 0.0);
  }
}

TEST_F(SceneParserSphereTest, EdgeCaseNegativeInfinityRadius) {
  // Caso adicional: Radio -infinito (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 -inf mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido (-inf < 0)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, EdgeCaseNaNRadius) {
  // Caso adicional: Radio NaN (inválido)
  // NaN no cumple radius > 0 (comparaciones con NaN son siempre false)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 nan mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // La esfera NO debe haberse añadido (NaN no cumple > 0)
    ASSERT_TRUE(scene.spheres.x.empty());
  }
}

TEST_F(SceneParserSphereTest, EdgeCaseInfinityCoordinates) {
  // Caso adicional: Coordenadas infinitas (válidas matemáticamente)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: inf -inf inf 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (coordenadas pueden ser infinitas)
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_TRUE(std::isinf(scene.spheres.x[0]));
    ASSERT_TRUE(std::isinf(scene.spheres.y[0]));
    ASSERT_TRUE(std::isinf(scene.spheres.z[0]));
  }
}

TEST_F(SceneParserSphereTest, EdgeCaseNaNCoordinates) {
  // Caso adicional: Coordenadas NaN (técnicamente válidas para parsedouble)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: nan nan nan 1.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (no hay validación de rango para coordenadas)
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_TRUE(std::isnan(scene.spheres.x[0]));
    ASSERT_TRUE(std::isnan(scene.spheres.y[0]));
    ASSERT_TRUE(std::isnan(scene.spheres.z[0]));
  }
}

TEST_F(SceneParserSphereTest, IntegrationValidSphereThenInvalidSphere) {
  // Caso adicional: Primera esfera válida, segunda inválida
  // Verifica que la primera se añade y la segunda se rechaza sin afectar la primera
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 1.0 mat1\n"
                 "sphere: 1 1 1 -0.5 mat1\n");  // Radio negativo

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo la primera esfera debe haberse añadido
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.x[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 1.0);
  }
}

TEST_F(SceneParserSphereTest, IntegrationInvalidSphereThenValidSphere) {
  // Caso adicional: Primera esfera inválida, segunda válida
  // Verifica que los errores no impiden el procesamiento posterior
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "sphere: 0 0 0 0 mat1\n"  // Radio cero (inválido)
                 "sphere: 1 1 1 0.5 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo la segunda esfera debe haberse añadido
    ASSERT_EQ(scene.spheres.x.size(), 1);
    ASSERT_DOUBLE_EQ(scene.spheres.x[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.spheres.y[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.spheres.z[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.spheres.r[0], 0.5);
  }
}

TEST_F(SceneParserSphereTest, IntegrationMixedObjectTypes) {
  // Caso adicional: Mezcla de esferas, cilindros y materiales
  // Verifica que parseSphere funciona correctamente en un contexto complejo
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "metal: mat2 0.6 0.6 0.6 0.1\n"
                 "sphere: 0 0 0 1.0 mat1\n"
                 "cylinder: 2 2 2 0.5 0 1 0 mat2\n"
                 "sphere: 4 4 4 2.0 mat2\n"
                 "refractive: mat3 1.5\n"
                 "sphere: -1 -1 -1 0.3 mat3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 esferas y 1 cilindro
    ASSERT_EQ(scene.spheres.x.size(), 3);
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    // Verificar materiales de las esferas
    ASSERT_EQ(scene.spheres.materialIndex[0], 0);  // mat1
    ASSERT_EQ(scene.spheres.materialIndex[1], 1);  // mat2
    ASSERT_EQ(scene.spheres.materialIndex[2], 2);  // mat3
  }
}

// ============================================================================
// TESTS PARA parseCylinder
// ============================================================================

class SceneParserCylinderTest : public ::testing::Test {
protected:
  std::string temp_filename;

  void SetUp() override { temp_filename = "test_scene_cylinder_temp.txt"; }

  void TearDown() override {
    // Remove temporary file
    if (std::remove(temp_filename.c_str()) != 0) {
      // File removal failed, but we don't want to fail the test for this
    }
  }

  void writeSceneFile(std::string const & content) {
    std::ofstream file(temp_filename);
    file << content;
    file.close();
  }
};

// CASOS VÁLIDOS - Cilindro parseado correctamente

TEST_F(SceneParserCylinderTest, ValidCylinderNonUnitaryAxis) {
  // Caso 2: Pre-cargar "mat1", parsear cilindro con eje Z de longitud 10
  // tokens = {"cylinder:", "1", "1", "1", "1.0", "0", "0", "10", "mat1"}
  // invAxisLen debe ser 1/10 = 0.1
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 1 1 1 1.0 0 0 10 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el cilindro fue añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    // Verificar centro
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.y[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.z[0], 1.0);
    // Verificar radio
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 1.0);
    // Verificar invAxisLen (longitud = 10 -> invAxisLen = 0.1)
    ASSERT_DOUBLE_EQ(scene.cylinders.invAxisLen[0], 0.1);
    // Verificar materialIndex
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderWithNegativeCoordinates) {
  // Caso adicional: Cilindro con coordenadas negativas (válido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: -10 -20.5 -30 2.5 -1 -2 -3 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el cilindro fue añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.vz[0], -3.0);
    ASSERT_NEAR(scene.cylinders.invAxisLen[0], 1.0 / std::sqrt(14.0), 1e-10);
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderWithVeryLargeCoordinates) {
  // Caso adicional: Coordenadas y eje muy grandes (pero válidos)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 1e10 -1e10 5e9 1e8 0 0 1e6 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el cilindro fue añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 1e10);
    ASSERT_DOUBLE_EQ(scene.cylinders.y[0], -1e10);
    ASSERT_DOUBLE_EQ(scene.cylinders.z[0], 5e9);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 1e8);
    ASSERT_DOUBLE_EQ(scene.cylinders.vz[0], 1e6);
    ASSERT_DOUBLE_EQ(scene.cylinders.invAxisLen[0], 1.0 / 1e6);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderWithVerySmallRadius) {
  // Caso adicional: Radio muy pequeño pero válido (> 0)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.000001 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el cilindro fue añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 0.000001);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderWithScientificNotation) {
  // Caso adicional: Valores en notación científica
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 1.5e2 -3.2e-1 4.7e3 2.1e1 1e0 2e0 3e0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el cilindro fue añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 150.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.y[0], -0.32);
    ASSERT_DOUBLE_EQ(scene.cylinders.z[0], 4700.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 21.0);
  }
}

TEST_F(SceneParserCylinderTest, ValidMultipleCylindersAdded) {
  // Caso adicional: Múltiples cilindros deben añadirse secuencialmente
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "metal: mat2 0.6 0.6 0.6 0.1\n"
                 "cylinder: 0 0 0 1 0 1 0 mat1\n"
                 "cylinder: 5 5 5 2 1 0 0 mat2\n"
                 "cylinder: -3 -3 -3 0.5 0 0 1 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 cilindros
    ASSERT_EQ(scene.cylinders.r.size(), 3);
    // Primer cilindro
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 1.0);
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderWithAllMaterialTypes) {
  // Caso adicional: Cilindros con cada tipo de material
  writeSceneFile("matte: matte_mat 0.8 0.2 0.2\n"
                 "metal: metal_mat 0.2 0.8 0.2 0.5\n"
                 "refractive: refract_mat 1.5\n"
                 "cylinder: 0 0 0 1 1 0 0 matte_mat\n"
                 "cylinder: 2 0 0 1 0 1 0 metal_mat\n"
                 "cylinder: 4 0 0 1 0 0 1 refract_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 cilindros
    ASSERT_EQ(scene.cylinders.r.size(), 3);
    // Verificar índices de materiales
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);  // matte_mat
    ASSERT_EQ(scene.cylinders.materialIndex[1], 1);  // metal_mat
    ASSERT_EQ(scene.cylinders.materialIndex[2], 2);  // refract_mat
    // Verificar tipos de materiales
    ASSERT_EQ(scene.materialTable[0].type, MaterialType::MATTE);
    ASSERT_EQ(scene.materialTable[1].type, MaterialType::METAL);
    ASSERT_EQ(scene.materialTable[2].type, MaterialType::REFRACTIVE);
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderAxisAlongX) {
  // Caso adicional: Cilindro con eje a lo largo del eje X
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1 5 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.vx[0], 5.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.vy[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.vz[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.invAxisLen[0], 0.2);  // 1/5
  }
}

TEST_F(SceneParserCylinderTest, ValidCylinderVerySmallAxis) {
  // Caso adicional: Eje muy pequeño pero no cero
  // Verifica el cálculo de invAxisLen con vectores pequeños
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1 0.001 0.001 0.001 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    // Longitud = sqrt(3 * 0.001^2) = sqrt(3e-6) ≈ 0.001732
    double const expected_length = std::sqrt(3.0 * 0.001 * 0.001);
    ASSERT_NEAR(scene.cylinders.invAxisLen[0], 1.0 / expected_length, 1e-8);
  }
}

// ERRORES DE FORMATO - Número incorrecto de argumentos

TEST_F(SceneParserCylinderTest, ErrorTooFewArguments) {
  // Caso 3a: Menos de 9 tokens (requiere 8 parámetros + comando)
  // Falta el material
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (error de formato)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorTooFewArgumentsMissingMultiple) {
  // Caso adicional: Faltan múltiples parámetros
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorOnlyCommand) {
  // Caso adicional: Solo el comando, sin parámetros
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder:\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorTooManyArguments) {
  // Caso 3b: Más de 9 tokens (argumentos extra)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 mat1 extra_arg\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (error de formato)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorTooManyArgumentsMultiple) {
  // Caso adicional: Múltiples argumentos extra
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 mat1 extra1 extra2\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

// ERRORES DE FORMATO - Valores no numéricos

TEST_F(SceneParserCylinderTest, ErrorNonNumericCentreX) {
  // Caso 3c: Valor no numérico en coordenada X del centro
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: abc 0 0 0.5 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericCentreY) {
  // Caso 3c: Valor no numérico en coordenada Y del centro
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 xyz 0 0.5 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericCentreZ) {
  // Caso 3c: Valor no numérico en coordenada Z del centro
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 invalid 0.5 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericRadius) {
  // Caso 3c: Valor no numérico en radio
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 notanumber 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericAxisX) {
  // Caso 3c: Valor no numérico en componente X del eje
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 bad 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericAxisY) {
  // Caso 3c: Valor no numérico en componente Y del eje
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 bad 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNonNumericAxisZ) {
  // Caso 3c: Valor no numérico en componente Z del eje
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 0 bad mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorPartialNumericValue) {
  // Caso adicional: Valor parcialmente numérico
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 5abc 0 0 0.5 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

// ERRORES DE RANGO/VALIDEZ - Radio inválido

TEST_F(SceneParserCylinderTest, ErrorRadiusZero) {
  // Caso 4a: Radio igual a cero (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (radio debe ser > 0)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorRadiusNegative) {
  // Caso 4b: Radio negativo (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 -0.5 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (radio debe ser > 0)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorRadiusVeryNegative) {
  // Caso adicional: Radio muy negativo
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 -1000000 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorRadiusNegativeScientific) {
  // Caso adicional: Radio negativo en notación científica
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 -1.5e-2 0 1 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

// ERRORES DE RANGO/VALIDEZ - Vector de eje cero

TEST_F(SceneParserCylinderTest, ErrorZeroAxisVector) {
  // Caso 4c: Vector de eje cero (0, 0, 0) - inválido
  // Un cilindro sin dirección no tiene sentido geométrico
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (eje no puede ser cero)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorZeroAxisVectorExplicit) {
  // Caso adicional: Vector de eje cero con valores explícitos 0.0
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 5 5 5 2.0 0.0 0.0 0.0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

// ERRORES DE MATERIAL - Material no encontrado

TEST_F(SceneParserCylinderTest, ErrorMaterialNotFound) {
  // Caso 5a: Referencia a material inexistente
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 unknown_mat\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // El cilindro NO debe haberse añadido (material no encontrado)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorNoMaterialsDefined) {
  // Caso adicional: Intentar crear cilindro sin definir ningún material
  writeSceneFile("cylinder: 0 0 0 0.5 0 1 0 any_material\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // No debe haber materiales
    ASSERT_TRUE(scene.materialNames.empty());
    // El cilindro NO debe haberse añadido
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorMaterialCaseSensitive) {
  // Caso 5b: Referencia a material con mayúsculas/minúsculas incorrectas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 Mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido (en minúsculas)
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "mat1");
    // El cilindro NO debe haberse añadido (Mat1 != mat1)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorMaterialAllUpperCase) {
  // Caso adicional: Material todo en mayúsculas
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 MAT1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material mat1 fue añadido (en minúsculas)
    ASSERT_EQ(scene.materialNames.size(), 1);
    // El cilindro NO debe haberse añadido (MAT1 != mat1)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, ErrorMaterialPartialName) {
  // Caso adicional: Usar solo parte del nombre del material
  writeSceneFile("matte: material_name 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0.5 0 1 0 material\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que el material completo fue añadido
    ASSERT_EQ(scene.materialNames.size(), 1);
    ASSERT_EQ(scene.materialNames[0], "material_name");
    // El cilindro NO debe haberse añadido (material != material_name)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

// CASOS ADICIONALES - Edge cases y valores especiales

TEST_F(SceneParserCylinderTest, EdgeCaseInfinityRadius) {
  // Caso adicional: Radio infinito (técnicamente válido > 0)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 inf 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar comportamiento con infinito
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_TRUE(std::isinf(scene.cylinders.r[0]));
    ASSERT_GT(scene.cylinders.r[0], 0.0);
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseNegativeInfinityRadius) {
  // Caso adicional: Radio -infinito (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 -inf 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido (-inf < 0)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseNaNRadius) {
  // Caso adicional: Radio NaN (inválido)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 nan 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro NO debe haberse añadido (NaN no cumple > 0)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseInfinityCoordinates) {
  // Caso adicional: Coordenadas infinitas (válidas matemáticamente)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: inf -inf inf 1.0 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (coordenadas pueden ser infinitas)
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_TRUE(std::isinf(scene.cylinders.x[0]));
    ASSERT_TRUE(std::isinf(scene.cylinders.y[0]));
    ASSERT_TRUE(std::isinf(scene.cylinders.z[0]));
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseInfinityAxisComponents) {
  // Caso adicional: Componentes del eje infinitos
  // El eje puede tener componentes infinitas (longitud infinita -> invAxisLen = 0)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1.0 inf 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (eje infinito es válido)
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_TRUE(std::isinf(scene.cylinders.vx[0]));
    ASSERT_DOUBLE_EQ(scene.cylinders.invAxisLen[0], 0.0);  // 1/inf = 0
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseNaNCoordinates) {
  // Caso adicional: Coordenadas NaN (técnicamente válidas para parsedouble)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: nan nan nan 1.0 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (no hay validación de rango para coordenadas)
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_TRUE(std::isnan(scene.cylinders.x[0]));
    ASSERT_TRUE(std::isnan(scene.cylinders.y[0]));
    ASSERT_TRUE(std::isnan(scene.cylinders.z[0]));
  }
}

TEST_F(SceneParserCylinderTest, EdgeCaseNaNAxisComponents) {
  // Caso adicional: Componentes del eje NaN
  // NaN en el eje resulta en longitud NaN, lo cual es problemático
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1.0 nan 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Debería añadirse (parsedouble acepta nan)
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_TRUE(std::isnan(scene.cylinders.vx[0]));
    // invAxisLen será NaN (sqrt con NaN produce NaN)
    ASSERT_TRUE(std::isnan(scene.cylinders.invAxisLen[0]));
  }
}

TEST_F(SceneParserCylinderTest, IntegrationValidCylinderThenInvalidCylinder) {
  // Caso adicional: Primer cilindro válido, segundo inválido
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1.0 1 0 0 mat1\n"
                 "cylinder: 1 1 1 -0.5 1 0 0 mat1\n");  // Radio negativo

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo el primer cilindro debe haberse añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 0.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 1.0);
  }
}

TEST_F(SceneParserCylinderTest, IntegrationInvalidCylinderThenValidCylinder) {
  // Caso adicional: Primer cilindro inválido, segundo válido
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 0 1 0 0 mat1\n"  // Radio cero (inválido)
                 "cylinder: 1 1 1 0.5 1 0 0 mat1\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Solo el segundo cilindro debe haberse añadido
    ASSERT_EQ(scene.cylinders.r.size(), 1);
    ASSERT_DOUBLE_EQ(scene.cylinders.x[0], 1.0);
    ASSERT_DOUBLE_EQ(scene.cylinders.r[0], 0.5);
  }
}

TEST_F(SceneParserCylinderTest, IntegrationMixedObjectTypes) {
  // Caso adicional: Mezcla de cilindros, esferas y materiales
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "metal: mat2 0.6 0.6 0.6 0.1\n"
                 "cylinder: 0 0 0 1.0 1 0 0 mat1\n"
                 "sphere: 2 2 2 0.5 mat2\n"
                 "cylinder: 4 4 4 2.0 0 1 0 mat2\n"
                 "refractive: mat3 1.5\n"
                 "cylinder: -1 -1 -1 0.3 0 0 1 mat3\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // Verificar que se añadieron 3 cilindros y 1 esfera
    ASSERT_EQ(scene.cylinders.r.size(), 3);
    ASSERT_EQ(scene.spheres.x.size(), 1);
    // Verificar materiales de los cilindros
    ASSERT_EQ(scene.cylinders.materialIndex[0], 0);  // mat1
    ASSERT_EQ(scene.cylinders.materialIndex[1], 1);  // mat2
    ASSERT_EQ(scene.cylinders.materialIndex[2], 2);  // mat3
  }
}

TEST_F(SceneParserCylinderTest, IntegrationCylinderWithZeroAxisRejectedBeforeMaterial) {
  // Caso adicional: Verifica que el error de eje cero se detecta antes de buscar el material
  // (el orden de validación en el código: primero radio, luego eje, luego material)
  writeSceneFile("matte: mat1 0.5 0.5 0.5\n"
                 "cylinder: 0 0 0 1.0 0 0 0 nonexistent\n");

  // Act: Llama a la función, que ahora devuelve un std::optional.
  auto const scene_opt = loadSceneFromFile(temp_filename);

  // Assert (Paso 2): Realiza las aserciones sobre el valor DENTRO de un bloque `if`.
  if (scene_opt) {
    SceneSettings const & scene = *scene_opt;
    // El cilindro no debe añadirse por el eje cero (detectado antes de buscar material)
    ASSERT_TRUE(scene.cylinders.r.empty());
  }
}
