#pragma once
#include <random>

class Rng {
    public:
    
    Rng(uint32_t seed) : m_eng(seed), m_seed(seed) {}
    
    int uniform_int(int low, int high) {
        std::uniform_int_distribution<int> dist(low, high);
        return dist(m_eng);
    }

    int m_seed;
    std::mt19937 m_eng;
};