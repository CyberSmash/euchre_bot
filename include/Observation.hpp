#pragma once

#include "Card.hpp"
#include "Phase.hpp"
#include "Hand.hpp"

struct Observation {

    Hand hand = {};
    Suit trump = {};
    Card lead = {};
    Card face_up_card = {};
    std::array<Card, 4> trick_cards = {};
    Phase phase = {};
    uint8_t maker_team = {};
    uint8_t player = {};
    uint8_t dealer = {};
    uint8_t num_played = {};
};