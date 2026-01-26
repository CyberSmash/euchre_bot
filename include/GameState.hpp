#pragma once

#include "Card.hpp"
#include "Defns.hpp"
#include <random>
#include <iostream>
#include "Hand.hpp"

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
    Hand hands[euchre::constants::num_players] {};

    void give_card_to(Card c, int player) {
        assert(player < euchre::constants::num_players);
        hands[player].give_card(c);
    }

    std::vector<Card> decode_hand(uint32_t hand) {
        std::vector<Card> cards(5);
        while (hand) {
            uint32_t bit = hand & -hand;
            int idx = std::countr_zero<uint32_t>(bit);
            cards.push_back(Card {idx});
        }
        return cards;
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