#pragma once

#include "Card.hpp"
#include "Defns.hpp"
#include <random>
#include "HandState.hpp"


class GameState {
    public:

    GameState() {
        hand_state.deck = euchre::constants::deck_reset;
        hand_state.maker_team = 0;
        hand_state.face_up_card = {euchre::constants::invalid_card};   
    }

    void reset_game(unsigned int seed) {
        eng.seed(seed);
    }

    enum class GameStatus : uint8_t {
        InProgress,
        GameOver,
    };

    GameStatus status = GameStatus::InProgress;
    uint8_t dealer = 0;
    uint8_t scores[2] {};
    HandState hand_state;
    std::mt19937 eng;

};