#include "utilities/random.hpp"
#include "utilities/vec3.hpp"
#include <cstdint>
#include <random>

RandomGenerator::RandomGenerator(std::uint64_t seed) : m_engine(seed) { }

// Genera un double en [0.0, 1.0)
double RandomGenerator::get_double() {
  // Esta distribución es la más común, así que la creamos aquí directamente.
  // Es eficiente crearla cada vez, pero para máxima claridad, está bien así.
  // Una optimización sería hacerla un miembro 'static' o de la clase.
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(m_engine);
}

// Genera un double en [min, max)
double RandomGenerator::get_double(double min, double max) {
  std::uniform_real_distribution<double> dist(min, max);
  return dist(m_engine);
}

// Genera un vector con componentes en [-1, 1)
Vec3 RandomGenerator::get_vector_minus1_to_1() {
  return {(get_double(-1.0, 1.0)), get_double(-1.0, 1.0), (get_double(-1.0, 1.0))};
}

// Genera un vector con componentes en [-range, range)
Vec3 RandomGenerator::get_vector_in_range(double range) {
  return {(get_double(-range, range)), get_double(-range, range), (get_double(-range, range))};
}

Vec3 RandomGenerator::get_unit_sphere() {
  while (true) {
    auto p = this->get_vector_minus1_to_1();
    if (p.length_squared() < 1) {
      return p;
    }
  }
}
