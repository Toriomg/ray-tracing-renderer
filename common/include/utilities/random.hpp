#ifndef RANDOM_HPP
#define RANDOM_HPP

#include "utilities/vec3.hpp"
#include <random>

class RandomGenerator{
    public:
    explicit RandomGenerator(std::uint64_t seed);
    double get_double();
    double get_double(double min, double max);
    float get_float(float min, float max);
    Vec3 get_vector_minus1_to_1();
    Vec3 get_vector_in_range(double range);
    Vec3 get_unit_sphere();
    
    private:
    std::mt19937_64 m_engine;
};

#endif
