#ifndef PARALLEL_RNG_HPP
#define PARALLEL_RNG_HPP

#include "utilities/random.hpp"
#include <algorithm>
#include <atomic>
#include <random>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/task_arena.h>
#include <thread>
#include <vector>

class ParallelRNGManager {
public:
  ParallelRNGManager(unsigned int base_seed_ray, unsigned int base_seed_material)
      : m_base_seed_ray(base_seed_ray), m_base_seed_material(base_seed_material) {
    determine_max_threads();
    generate_seed_vectors();
  }

  RandomGenerator & get_ray_rng() { return m_ray_rngs.local(); }

  RandomGenerator & get_material_rng() { return m_material_rngs.local(); }

private:
  unsigned int m_base_seed_ray;
  unsigned int m_base_seed_material;

  std::vector<std::uint64_t> m_ray_seeds;
  std::vector<std::uint64_t> m_material_seeds;

  static inline std::atomic<std::size_t> s_thread_counter{0};

  std::size_t m_max_threads{0};  // Inicialización por defecto aquí

  void determine_max_threads() {
    // static_cast para evitar warning de conversión de signo
    m_max_threads = static_cast<std::size_t>(tbb::this_task_arena::max_concurrency());

    if (m_max_threads == 0) {
      m_max_threads = static_cast<std::size_t>(std::thread::hardware_concurrency());
    }

    if (m_max_threads == 0) {
      m_max_threads = 32U;  // 'U' para unsigned literal
    }
  }

  void generate_seed_vectors() {
    m_ray_seeds.resize(m_max_threads);
    m_material_seeds.resize(m_max_threads);

    // std::mt19937_64 (enunciado dice mt19934_64 por error tipográfico)
    std::mt19937_64 ray_seed_gen(static_cast<std::uint64_t>(m_base_seed_ray));
    std::mt19937_64 material_seed_gen(static_cast<std::uint64_t>(m_base_seed_material));

    // Usar std::ranges::generate EXACTAMENTE como en Listado 6 del enunciado
    // Si tienes C++20, usa esto:
    std::ranges::generate(m_ray_seeds, ray_seed_gen);
    std::ranges::generate(m_material_seeds, material_seed_gen);
  }

  tbb::enumerable_thread_specific<RandomGenerator> m_ray_rngs{[this]() {
    std::size_t thread_id = s_thread_counter.fetch_add(1);
    return RandomGenerator(m_ray_seeds[thread_id % m_ray_seeds.size()]);
  }};

  tbb::enumerable_thread_specific<RandomGenerator> m_material_rngs{[this]() {
    std::size_t thread_id = s_thread_counter.fetch_add(1);
    return RandomGenerator(m_material_seeds[thread_id % m_material_seeds.size()]);
  }};
};

#endif
