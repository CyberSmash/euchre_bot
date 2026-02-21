#pragma once
#include <cstdint>
#include <random>
#include "Card.hpp"

template <std::unsigned_integral T>
inline T pick_random_bit(T mask, std::mt19937& rng) {
    assert(mask != 0);
    
    // Get the number of cards left in the deck.
    int cards_left = std::popcount(mask);
    std::uniform_int_distribution<int> dist(0, cards_left - 1);
    int k = dist(rng);
    T m = mask;
    while(k--) {
        m &= (m - 1);
    }

    return static_cast<T>(std::countr_zero(m));
}

Card draw_card(uint32_t& remaining_cards, std::mt19937& rng);