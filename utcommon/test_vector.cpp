#include "utilities/vec3.hpp"
#include <cmath>
#include <gtest/gtest.h>

// ============================================================================
// FIXTURE DE GOOGLETEST PARA Vec3
// ============================================================================

class VectorTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Inicialización común si es necesaria
  }

  void TearDown() override {
    // Limpieza común si es necesaria
  }
};

// ============================================================================
// TESTS DE CONSTRUCTORES
// ============================================================================

TEST_F(VectorTest, DefaultConstructor) {
  Vec3 const v;

  ASSERT_DOUBLE_EQ(v.x, 0.0);
  ASSERT_DOUBLE_EQ(v.y, 0.0);
  ASSERT_DOUBLE_EQ(v.z, 0.0);
}

TEST_F(VectorTest, ValueConstructor) {
  Vec3 const v(1.0, 2.0, 3.0);

  ASSERT_DOUBLE_EQ(v.x, 1.0);
  ASSERT_DOUBLE_EQ(v.y, 2.0);
  ASSERT_DOUBLE_EQ(v.z, 3.0);
}

// ============================================================================
// TESTS DE OPERADORES ARITMÉTICOS UNARIOS Y BINARIOS
// ============================================================================

TEST_F(VectorTest, UnaryNegation) {
  Vec3 const v1(1.0, 2.0, 3.0);
  Vec3 const result = -v1;

  ASSERT_DOUBLE_EQ(result.x, -1.0);
  ASSERT_DOUBLE_EQ(result.y, -2.0);
  ASSERT_DOUBLE_EQ(result.z, -3.0);
}

TEST_F(VectorTest, BinaryAddition) {
  Vec3 const v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  Vec3 const result = v1 + v2;

  ASSERT_DOUBLE_EQ(result.x, 5.0);
  ASSERT_DOUBLE_EQ(result.y, 7.0);
  ASSERT_DOUBLE_EQ(result.z, 9.0);
}

TEST_F(VectorTest, BinarySubtraction) {
  Vec3 const v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  Vec3 const result = v1 - v2;

  ASSERT_DOUBLE_EQ(result.x, -3.0);
  ASSERT_DOUBLE_EQ(result.y, -3.0);
  ASSERT_DOUBLE_EQ(result.z, -3.0);
}

TEST_F(VectorTest, BinaryMultiplicationVecVec) {
  // Multiplicación por componentes (Hadamard product)
  Vec3 const v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  Vec3 const result = v1 * v2;

  ASSERT_DOUBLE_EQ(result.x, 4.0);   // 1 * 4
  ASSERT_DOUBLE_EQ(result.y, 10.0);  // 2 * 5
  ASSERT_DOUBLE_EQ(result.z, 18.0);  // 3 * 6
}

TEST_F(VectorTest, BinaryMultiplicationVecScalar) {
  Vec3 const v1(1.0, 2.0, 3.0);

  // Vec3 * scalar
  Vec3 const result1 = v1 * 2.0;
  ASSERT_DOUBLE_EQ(result1.x, 2.0);
  ASSERT_DOUBLE_EQ(result1.y, 4.0);
  ASSERT_DOUBLE_EQ(result1.z, 6.0);

  // scalar * Vec3
  Vec3 const result2 = 2.0 * v1;
  ASSERT_DOUBLE_EQ(result2.x, 2.0);
  ASSERT_DOUBLE_EQ(result2.y, 4.0);
  ASSERT_DOUBLE_EQ(result2.z, 6.0);
}

TEST_F(VectorTest, BinaryDivisionScalar) {
  Vec3 const v1(2.0, 4.0, 6.0);
  Vec3 const result = v1 / 2.0;

  ASSERT_DOUBLE_EQ(result.x, 1.0);
  ASSERT_DOUBLE_EQ(result.y, 2.0);
  ASSERT_DOUBLE_EQ(result.z, 3.0);
}

// ============================================================================
// TESTS DE OPERADORES DE ASIGNACIÓN
// ============================================================================

TEST_F(VectorTest, AssignmentAddition) {
  Vec3 v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  v1 += v2;

  ASSERT_DOUBLE_EQ(v1.x, 5.0);
  ASSERT_DOUBLE_EQ(v1.y, 7.0);
  ASSERT_DOUBLE_EQ(v1.z, 9.0);
}

TEST_F(VectorTest, AssignmentSubtraction) {
  Vec3 v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  v1 -= v2;

  ASSERT_DOUBLE_EQ(v1.x, -3.0);
  ASSERT_DOUBLE_EQ(v1.y, -3.0);
  ASSERT_DOUBLE_EQ(v1.z, -3.0);
}

TEST_F(VectorTest, AssignmentMultiplication) {
  Vec3 v1(1.0, 2.0, 3.0);
  v1 *= 2.0;

  ASSERT_DOUBLE_EQ(v1.x, 2.0);
  ASSERT_DOUBLE_EQ(v1.y, 4.0);
  ASSERT_DOUBLE_EQ(v1.z, 6.0);
}

TEST_F(VectorTest, AssignmentDivision) {
  Vec3 v1(2.0, 4.0, 6.0);
  v1 /= 2.0;

  ASSERT_DOUBLE_EQ(v1.x, 1.0);
  ASSERT_DOUBLE_EQ(v1.y, 2.0);
  ASSERT_DOUBLE_EQ(v1.z, 3.0);
}

// ============================================================================
// TESTS DE FUNCIONES MIEMBRO (UTILIDAD)
// ============================================================================

TEST_F(VectorTest, LengthSquared) {
  Vec3 const v1(1.0, 2.0, 3.0);
  double const result = v1.length_squared();

  // 1*1 + 2*2 + 3*3 = 1 + 4 + 9 = 14
  ASSERT_DOUBLE_EQ(result, 14.0);
}

TEST_F(VectorTest, Length) {
  Vec3 const v1(1.0, 2.0, 3.0);
  double const result = v1.length();

  // sqrt(14) ≈ 3.7416573867739413
  ASSERT_DOUBLE_EQ(result, std::sqrt(14.0));
}

TEST_F(VectorTest, IsNearZero) {
  // Vector muy pequeño (debe ser considerado "near zero")
  Vec3 const v1(1e-9, 1e-9, 1e-9);
  ASSERT_TRUE(v1.is_near_zero());

  // Vector con al menos una componente significativa
  Vec3 const v2(1.0, 0.0, 0.0);
  ASSERT_FALSE(v2.is_near_zero());

  // Vector exactamente cero
  Vec3 const v3(0.0, 0.0, 0.0);
  ASSERT_TRUE(v3.is_near_zero());

  // Vector justo en el límite (1e-8 es el threshold)
  Vec3 const v4(1e-7, 0.0, 0.0);
  ASSERT_FALSE(v4.is_near_zero());
}

// ============================================================================
// TESTS DE FUNCIONES LIBRES (GEOMETRÍA)
// ============================================================================

TEST_F(VectorTest, DotProduct) {
  Vec3 const v1(1.0, 2.0, 3.0);
  Vec3 const v2(4.0, 5.0, 6.0);
  double const result = dot(v1, v2);

  // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
  ASSERT_DOUBLE_EQ(result, 32.0);
}

TEST_F(VectorTest, CrossProduct) {
  // Caso clásico: eje X × eje Y = eje Z
  Vec3 const v1(1.0, 0.0, 0.0);
  Vec3 const v2(0.0, 1.0, 0.0);
  Vec3 const result = cross(v1, v2);

  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 1.0);

  // Caso general
  Vec3 const v3(1.0, 2.0, 3.0);
  Vec3 const v4(4.0, 5.0, 6.0);
  Vec3 const result2 = cross(v3, v4);

  // v3 × v4 = (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4)
  //         = (12 - 15, 12 - 6, 5 - 8)
  //         = (-3, 6, -3)
  ASSERT_DOUBLE_EQ(result2.x, -3.0);
  ASSERT_DOUBLE_EQ(result2.y, 6.0);
  ASSERT_DOUBLE_EQ(result2.z, -3.0);
}

TEST_F(VectorTest, Normalize) {
  // Normalizar un vector del eje Y con longitud 5
  Vec3 const v1(0.0, 5.0, 0.0);
  Vec3 const result = v1.normalize();

  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 1.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);

  // Normalizar un vector general
  Vec3 const v2(3.0, 4.0, 0.0);
  Vec3 const result2 = v2.normalize();

  // Longitud = sqrt(9 + 16) = sqrt(25) = 5
  // Normalizado = (3/5, 4/5, 0) = (0.6, 0.8, 0)
  ASSERT_DOUBLE_EQ(result2.x, 0.6);
  ASSERT_DOUBLE_EQ(result2.y, 0.8);
  ASSERT_DOUBLE_EQ(result2.z, 0.0);

  // Verificar que la longitud del vector normalizado es 1
  ASSERT_DOUBLE_EQ(result2.length(), 1.0);
}

// ============================================================================
// TESTS DE CASOS BORDE
// ============================================================================


TEST_F(VectorTest, NormalizeZeroVector) {
  // Según el código:
  // len_sq = 0.0 (para vector cero)
  // len_sq > epsilon es false
  // Retorna *this (el vector cero)
  Vec3 const v1(0.0, 0.0, 0.0);
  Vec3 const result = v1.normalize();

  // El código retorna el vector original cuando len_sq <= epsilon
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 0.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);

  // Nota: Este comportamiento es seguro (retorna vector cero)
  // No genera NaN como se esperaba en las instrucciones
}

// ============================================================================
// TESTS ADICIONALES DE CASOS BORDE
// ============================================================================

TEST_F(VectorTest, ReflectVector) {
  // Probar la función reflect
  // v - 2 * dot(v, n) * n
  Vec3 const v(1.0, -1.0, 0.0);  // Vector incidente
  Vec3 const n(0.0, 1.0, 0.0);   // Normal (superficie horizontal)
  Vec3 const result = reflect(v, n);

  // dot(v, n) = 1*0 + (-1)*1 + 0*0 = -1
  // reflect = (1, -1, 0) - 2*(-1)*(0, 1, 0)
  //         = (1, -1, 0) + (0, 2, 0)
  //         = (1, 1, 0)
  ASSERT_DOUBLE_EQ(result.x, 1.0);
  ASSERT_DOUBLE_EQ(result.y, 1.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

TEST_F(VectorTest, ComponentPerpendicular) {
  // Probar component_perpendicular
  // v - dot(v, u) * u
  Vec3 const v(1.0, 1.0, 0.0);
  Vec3 const u(1.0, 0.0, 0.0);  // Vector unitario en eje X
  Vec3 const result = component_perpendicular(v, u);

  // dot(v, u) = 1*1 + 1*0 + 0*0 = 1
  // result = (1, 1, 0) - 1*(1, 0, 0) = (0, 1, 0)
  ASSERT_DOUBLE_EQ(result.x, 0.0);
  ASSERT_DOUBLE_EQ(result.y, 1.0);
  ASSERT_DOUBLE_EQ(result.z, 0.0);
}

TEST_F(VectorTest, MinMaxFunctions) {
  Vec3 const v1(1.0, 5.0, 3.0);
  Vec3 const v2(4.0, 2.0, 6.0);

  // Función min
  Vec3 const result_min = min(v1, v2);
  ASSERT_DOUBLE_EQ(result_min.x, 1.0);
  ASSERT_DOUBLE_EQ(result_min.y, 2.0);
  ASSERT_DOUBLE_EQ(result_min.z, 3.0);

  // Función max
  Vec3 const result_max = max(v1, v2);
  ASSERT_DOUBLE_EQ(result_max.x, 4.0);
  ASSERT_DOUBLE_EQ(result_max.y, 5.0);
  ASSERT_DOUBLE_EQ(result_max.z, 6.0);
}

TEST_F(VectorTest, RefractFunction) {
  // Probar la función refract (Ley de Snell)
  Vec3 const uv(0.0, -1.0, 0.0);            // Vector unitario hacia abajo
  Vec3 const n(0.0, 1.0, 0.0);              // Normal hacia arriba
  double const etai_over_etat = 1.0 / 1.5;  // Aire -> Vidrio

  Vec3 const result = refract(uv, n, etai_over_etat);

  // El resultado debe estar en el mismo plano y refractado
  // Verificar que no es NaN
  ASSERT_FALSE(std::isnan(result.x));
  ASSERT_FALSE(std::isnan(result.y));
  ASSERT_FALSE(std::isnan(result.z));

  // Verificar que la componente Y es negativa (refractado hacia abajo)
  ASSERT_LT(result.y, 0.0);
}

// ============================================================================
// TESTS DE TIPO ALIAS (Color y Point3)
// ============================================================================

TEST_F(VectorTest, TypeAliases) {
  // Verificar que Color y Point3 son equivalentes a Vec3
  Color const c(1.0, 0.5, 0.0);
  Point3 const p(1.0, 2.0, 3.0);

  ASSERT_DOUBLE_EQ(c.x, 1.0);
  ASSERT_DOUBLE_EQ(c.y, 0.5);
  ASSERT_DOUBLE_EQ(c.z, 0.0);

  ASSERT_DOUBLE_EQ(p.x, 1.0);
  ASSERT_DOUBLE_EQ(p.y, 2.0);
  ASSERT_DOUBLE_EQ(p.z, 3.0);

  // Verificar que se pueden usar operaciones
  Color const c2 = c * 2.0;
  ASSERT_DOUBLE_EQ(c2.x, 2.0);
  ASSERT_DOUBLE_EQ(c2.y, 1.0);
  ASSERT_DOUBLE_EQ(c2.z, 0.0);
}
