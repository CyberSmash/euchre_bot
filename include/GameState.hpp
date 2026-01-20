#pragma once

#include "Card.hpp"
#include "Defns.hpp"
class GameState {
    public:

    GameState() {
        deck = euchre::constants::deck_reset;
        maker_team = 0;
        face_up_card = 0;
        
    }

    uint32_t deck = euchre::constants::deck_reset;
    int face_up_card = {};
    int current_player = {};
    int maker_team =  {};
    Suit trump = {};


};