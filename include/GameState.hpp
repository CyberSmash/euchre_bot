#pragma once

#include "Card.hpp"
#include "Defns.hpp"
#include <random>
#include <iostream>

enum class Phase : uint8_t { 
    Deal,
    BidRound1,
    DealerPickupDiscard,
    BidRound2,
    PlayTrick,
    HandOver,

};


struct HandState {
    Phase phase;
    unsigned int hands[euchre::constants::num_players];

    inline constexpr void give_card(Card c, uint8_t player) {
        hands[player] |= (1 << c);
    }

    inline constexpr void show_hand(uint8_t player) {
        
    }

};


class GameState {
    public:

    GameState() {
        deck = euchre::constants::deck_reset;
        maker_team = 0;
        face_up_card = 0;
        
    }

    void reset_game(unsigned int seed) {
        eng.seed(seed);
    }

    uint32_t deck = euchre::constants::deck_reset;
    int face_up_card = {};
    int current_player = {};
    int maker_team =  {};
    Suit trump = {};
    bool game_over = false;
    HandState hand_state;
    std::mt19937 eng;

};