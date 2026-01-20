#include "Deck.hpp"
#include "Card.hpp"
#include <bit>

Card draw_card(uint32_t& remaining_cards, std::mt19937& rng) {
    // Get the number of cards left in the deck.
    int cards_left = std::popcount(remaining_cards);
    std::uniform_int_distribution<int> dist(0, cards_left - 1);
    int k = dist(rng);
    unsigned int m = remaining_cards;
    while(k--) {
        m &= (m - 1);
    }

    Card c = std::countr_zero(m);
    remaining_cards &= ~(1u << c);
    return c;
}