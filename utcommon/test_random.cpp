#include "utilities/random.hpp"
#include "utilities/vec3.hpp"
#include <cstdint>
#include <gtest/gtest.h>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA RandomGenerator
// ============================================================================

class RandomTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza común si es necesaria
  }
};

// ============================================================================
// TESTS PARA RandomGenerator
// ============================================================================

// Test 1: Determinismo - misma semilla produce mismos resultados
TEST_F(RandomTest, DeterminismCheck) {
  // Configuración: dos generadores con la misma semilla
  std::uint64_t const seed = 12'345;
  RandomGenerator rng1(seed);
  RandomGenerator rng2(seed);

  // Verificar que get_double() produce los mismos valores
  double const val1 = rng1.get_double();
  double const val2 = rng2.get_double();
  ASSERT_DOUBLE_EQ(val1, val2) << "get_double() debe ser determinista con la misma semilla";

  // Verificar que get_double(min, max) produce los mismos valores
  double const val3 = rng1.get_double(-5.0, 5.0);
  double const val4 = rng2.get_double(-5.0, 5.0);
  ASSERT_DOUBLE_EQ(val3, val4) << "get_double(min, max) debe ser determinista con la misma semilla";

  // Verificar que get_vector_minus1_to_1() produce los mismos vectores
  Vec3 const vec1 = rng1.get_vector_minus1_to_1();
  Vec3 const vec2 = rng2.get_vector_minus1_to_1();
  ASSERT_DOUBLE_EQ(vec1.x, vec2.x) << "get_vector_minus1_to_1() componente X debe ser determinista";
  ASSERT_DOUBLE_EQ(vec1.y, vec2.y) << "get_vector_minus1_to_1() componente Y debe ser determinista";
  ASSERT_DOUBLE_EQ(vec1.z, vec2.z) << "get_vector_minus1_to_1() componente Z debe ser determinista";

  // Verificar que get_unit_sphere() produce los mismos vectores
  Vec3 const vec3 = rng1.get_unit_sphere();
  Vec3 const vec4 = rng2.get_unit_sphere();
  ASSERT_DOUBLE_EQ(vec3.x, vec4.x) << "get_unit_sphere() componente X debe ser determinista";
  ASSERT_DOUBLE_EQ(vec3.y, vec4.y) << "get_unit_sphere() componente Y debe ser determinista";
  ASSERT_DOUBLE_EQ(vec3.z, vec4.z) << "get_unit_sphere() componente Z debe ser determinista";
}

// Test 2: Verificar que get_double() retorna valores en [0.0, 1.0)
TEST_F(RandomTest, GetDouble0To1Range) {
  // Configuración
  RandomGenerator rng(1);

  // Generar muchos valores y verificar que están en el rango correcto
  for (int i = 0; i < 1'000; ++i) {
    double const val = rng.get_double();

    // Verificar límite inferior: val >= 0.0
    ASSERT_GE(val, 0.0) << "get_double() debe retornar valores >= 0.0";

    // Verificar límite superior: val < 1.0
    ASSERT_LT(val, 1.0) << "get_double() debe retornar valores < 1.0";
  }
}

// Test 3: Verificar que get_double(min, max) retorna valores en [min, max)
TEST_F(RandomTest, GetDoubleMinToMaxRange) {
  // Configuración
  RandomGenerator rng(2);
  double const min = -10.0;
  double const max = 10.0;

  // Generar muchos valores y verificar que están en el rango correcto
  for (int i = 0; i < 1'000; ++i) {
    double const val = rng.get_double(min, max);

    // Verificar límite inferior: val >= min
    ASSERT_GE(val, min) << "get_double(min, max) debe retornar valores >= min";

    // Verificar límite superior: val < max
    ASSERT_LT(val, max) << "get_double(min, max) debe retornar valores < max";
  }
}

// Test 4: Verificar que get_double(min, max) funciona con rangos diferentes
TEST_F(RandomTest, GetDoubleMinToMaxDifferentRanges) {
  RandomGenerator rng(3);

  // Rango positivo grande
  for (int i = 0; i < 100; ++i) {
    double const val = rng.get_double(100.0, 200.0);
    ASSERT_GE(val, 100.0);
    ASSERT_LT(val, 200.0);
  }

  // Rango negativo
  for (int i = 0; i < 100; ++i) {
    double const val = rng.get_double(-50.0, -10.0);
    ASSERT_GE(val, -50.0);
    ASSERT_LT(val, -10.0);
  }

  // Rango muy pequeño
  for (int i = 0; i < 100; ++i) {
    double const val = rng.get_double(0.0, 0.001);
    ASSERT_GE(val, 0.0);
    ASSERT_LT(val, 0.001);
  }
}

// Test 5: Verificar que get_vector_minus1_to_1() retorna vectores con componentes en [-1.0, 1.0)
TEST_F(RandomTest, GetVectorMinus1To1Range) {
  // Configuración
  RandomGenerator rng(4);

  // Generar muchos vectores y verificar que los componentes están en el rango correcto
  for (int i = 0; i < 1'000; ++i) {
    Vec3 const v = rng.get_vector_minus1_to_1();

    // Verificar componente X
    ASSERT_GE(v.x, -1.0) << "get_vector_minus1_to_1() componente X debe ser >= -1.0";
    ASSERT_LT(v.x, 1.0) << "get_vector_minus1_to_1() componente X debe ser < 1.0";

    // Verificar componente Y
    ASSERT_GE(v.y, -1.0) << "get_vector_minus1_to_1() componente Y debe ser >= -1.0";
    ASSERT_LT(v.y, 1.0) << "get_vector_minus1_to_1() componente Y debe ser < 1.0";

    // Verificar componente Z
    ASSERT_GE(v.z, -1.0) << "get_vector_minus1_to_1() componente Z debe ser >= -1.0";
    ASSERT_LT(v.z, 1.0) << "get_vector_minus1_to_1() componente Z debe ser < 1.0";
  }
}

// Test 6: Verificar que get_vector_in_range() retorna vectores con componentes en [-range, range)
TEST_F(RandomTest, GetVectorInRangeRange) {
  // Configuración
  RandomGenerator rng(5);
  double const range = 5.0;

  // Generar muchos vectores y verificar que los componentes están en el rango correcto
  for (int i = 0; i < 1'000; ++i) {
    Vec3 const v = rng.get_vector_in_range(range);

    // Verificar componente X
    ASSERT_GE(v.x, -range) << "get_vector_in_range() componente X debe ser >= -range";
    ASSERT_LT(v.x, range) << "get_vector_in_range() componente X debe ser < range";

    // Verificar componente Y
    ASSERT_GE(v.y, -range) << "get_vector_in_range() componente Y debe ser >= -range";
    ASSERT_LT(v.y, range) << "get_vector_in_range() componente Y debe ser < range";

    // Verificar componente Z
    ASSERT_GE(v.z, -range) << "get_vector_in_range() componente Z debe ser >= -range";
    ASSERT_LT(v.z, range) << "get_vector_in_range() componente Z debe ser < range";
  }
}

// Test 7: Verificar que get_unit_sphere() retorna vectores dentro de la esfera unitaria
TEST_F(RandomTest, GetUnitSphereRange) {
  // Configuración
  RandomGenerator rng(6);

  // Generar muchos vectores y verificar que están dentro de la esfera unitaria
  for (int i = 0; i < 1'000; ++i) {
    Vec3 const v = rng.get_unit_sphere();

    // Verificar que el vector está DENTRO de la esfera unitaria
    // length_squared() debe ser ESTRICTAMENTE menor que 1.0
    ASSERT_LT(v.length_squared(), 1.0)
        << "get_unit_sphere() debe retornar vectores con length_squared() < 1.0";
  }
}

// Test 8: Verificar que semillas diferentes producen secuencias diferentes
TEST_F(RandomTest, DifferentSeedsDifferentSequences) {
  // Configuración: dos generadores con semillas diferentes
  RandomGenerator rng1(100);
  RandomGenerator rng2(200);

  // Generar valores y verificar que son diferentes
  double const val1 = rng1.get_double();
  double const val2 = rng2.get_double();

  // Nota: Técnicamente, con probabilidad extremadamente baja, podrían ser iguales
  // pero con semillas diferentes y MT19937, esto no debería ocurrir en la práctica
  ASSERT_NE(val1, val2) << "Semillas diferentes deben producir valores diferentes";

  Vec3 const vec1 = rng1.get_unit_sphere();
  Vec3 const vec2 = rng2.get_unit_sphere();

  // Al menos uno de los componentes debería ser diferente
  bool const are_different = (vec1.x != vec2.x) or (vec1.y != vec2.y) or (vec1.z != vec2.z);
  ASSERT_TRUE(are_different) << "Semillas diferentes deben producir vectores diferentes";
}

// Test 9: Verificar get_vector_in_range con diferentes rangos

namespace {

  // --- FUNCIÓN AUXILIAR DE VERIFICACIÓN ---
  // Su única responsabilidad es verificar que cada componente de un Vec3
  // está dentro del rango simétrico esperado [-range, range).
  void assertVectorInRange(Vec3 const & v, double range) {
    // Usamos EXPECT en lugar de ASSERT aquí para que si una aserción falla,
    // la prueba continúe y verifique los otros componentes, dando un reporte más completo.
    EXPECT_GE(v.x, -range) << "Componente X fuera de rango (demasiado bajo)";
    EXPECT_LT(v.x, range) << "Componente X fuera de rango (demasiado alto)";
    EXPECT_GE(v.y, -range) << "Componente Y fuera de rango (demasiado bajo)";
    EXPECT_LT(v.y, range) << "Componente Y fuera de rango (demasiado alto)";
    EXPECT_GE(v.z, -range) << "Componente Z fuera de rango (demasiado bajo)";
    EXPECT_LT(v.z, range) << "Componente Z fuera de rango (demasiado alto)";
  }

}  // namespace

// --- PRUEBA PRINCIPAL (AHORA SIMPLIFICADA) ---
TEST_F(RandomTest, GetVectorInRangeDifferentRanges) {
  // 1. Arrange
  RandomGenerator rng(7);

  // 2. Act & Assert

  // Rango pequeño
  for (int i = 0; i < 100; ++i) {
    Vec3 const v = rng.get_vector_in_range(0.1);
    // La lógica de verificación ahora está en una sola llamada a la función auxiliar.
    assertVectorInRange(v, 0.1);
  }

  // Rango grande
  for (int i = 0; i < 100; ++i) {
    Vec3 const v = rng.get_vector_in_range(100.0);
    // Se reutiliza la misma función auxiliar con un rango diferente.
    assertVectorInRange(v, 100.0);
  }
}

// Test 10: Verificar la secuencia completa con determinismo
TEST_F(RandomTest, FullSequenceDeterminism) {
  // Configuración: dos generadores con la misma semilla
  std::uint64_t const seed = 99'999;
  RandomGenerator rng1(seed);
  RandomGenerator rng2(seed);

  // Generar una secuencia de diferentes llamadas y verificar determinismo
  for (int i = 0; i < 10; ++i) {
    double const d1 = rng1.get_double();
    double const d2 = rng2.get_double();
    ASSERT_DOUBLE_EQ(d1, d2);

    Vec3 const v1 = rng1.get_vector_minus1_to_1();
    Vec3 const v2 = rng2.get_vector_minus1_to_1();
    ASSERT_DOUBLE_EQ(v1.x, v2.x);
    ASSERT_DOUBLE_EQ(v1.y, v2.y);
    ASSERT_DOUBLE_EQ(v1.z, v2.z);

    double const d3 = rng1.get_double(0.0, 10.0);
    double const d4 = rng2.get_double(0.0, 10.0);
    ASSERT_DOUBLE_EQ(d3, d4);
  }
}
