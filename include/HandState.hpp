#pragma once
#include <cstdint>
#include <vector>
#include "Phase.hpp"
#include "Card.hpp"
#include "Observation.hpp"

struct HandState {
    Phase       phase = Phase::Deal;
    Hand        hands[euchre::constants::num_players] {};
    uint32_t    deck = euchre::constants::deck_reset;
    Card        face_up_card = Card{euchre::constants::invalid_card};
    uint8_t     maker_team =  0;
    Suit        trump = Suit::None;
    Card        lead_card = euchre::constants::invalid_card;
    int         num_played = 0;
    bool        stick_the_dealer = false;
    uint8_t     maker_player = 0;
    bool        going_alone = false;
    uint8_t     tricks_won[2] = {};
    uint8_t     lead_player = 0;
    uint8_t     tricks_played = 0;
    std::array<Card, 4> trick_cards;

    HandState() = default;

    void reset() {
        *this = HandState {};
    }

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

    Observation generate_observation(uint8_t current_player, uint8_t dealer_idx) {
        assert(current_player < euchre::constants::num_players);
        Observation obs = {
            .hand = hands[current_player],
            .trump = trump,
            .lead = lead_card,
            .face_up_card = face_up_card,
            .trick_cards = trick_cards,
            .phase = phase,
            .maker_team = maker_team,
            .player = current_player,
            .dealer = dealer_idx,
            .num_played = static_cast<uint8_t>(num_played),
        };

        return obs;
    }
};
