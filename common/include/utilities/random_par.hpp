#ifndef PARALLEL_RNG_HPP
#define PARALLEL_RNG_HPP

#include "utilities/random.hpp"
#include <atomic>
#include <tbb/enumerable_thread_specific.h>

class ParallelRNGManager {
public:
  ParallelRNGManager(unsigned int base_seed_ray, unsigned int base_seed_material)
      : m_base_seed_ray(base_seed_ray), m_base_seed_material(base_seed_material) { }

  // Acceso a los RNG locales del hilo
  RandomGenerator & get_ray_rng() { return m_ray_rngs.local(); }

  RandomGenerator & get_material_rng() { return m_material_rngs.local(); }

private:
  unsigned int m_base_seed_ray;
  unsigned int m_base_seed_material;

  // Aseguramos semillas únicas en cada hilo
  static inline std::atomic<unsigned int> s_ray_counter{0};
  static inline std::atomic<unsigned int> s_material_counter{0};

  // Solo se crean generadoes de numeros aleatorios para los hilos que existen
  tbb::enumerable_thread_specific<RandomGenerator> m_ray_rngs{[this]() {
    unsigned int thread_seed = m_base_seed_ray + s_ray_counter.fetch_add(1);
    return RandomGenerator(thread_seed);
  }};

  tbb::enumerable_thread_specific<RandomGenerator> m_material_rngs{[this]() {
    unsigned int thread_seed = m_base_seed_material + s_material_counter.fetch_add(1);
    return RandomGenerator(thread_seed);
  }};
};

#endif
