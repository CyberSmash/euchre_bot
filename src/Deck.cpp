#include "Deck.hpp"
#include "Card.hpp"
#include <bit>

Card draw_card(uint32_t& remaining_cards, std::mt19937& rng) {

    uint32_t bit = pick_random_bit(remaining_cards, rng);
    Card c {bit};
    remaining_cards &= ~(1u << c);
    return c;
}