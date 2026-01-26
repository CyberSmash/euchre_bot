#pragma once

#include <cstdint>
#include "Card.hpp"
#include "GameState.hpp"

struct Observation {
    Hand hand;
    Suit trump;
    Card lead;
    uint32_t trick_cards;
    Phase phase;
};