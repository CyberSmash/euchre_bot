#include "Rng.hpp"
#include "GameState.hpp"
#include <random>
#include <exception>
#include "Deck.hpp"

class Env {

    public:
    Env(unsigned int seed) : eng(seed), state() {}

    void deal() {
        for (uint8_t i = 0; i < euchre::constants::num_players; i++) {
            for (uint8_t j = 0; j < 5; j++) {
                Card c = draw_card(state.deck, state.eng);
                state.hand_state.give_card(c, i);
            }
        }
    }

    void step() {
        switch(state.hand_state.phase) {
            case Phase::Deal:
                deal();
                state.hand_state.phase = Phase::BidRound1;
                break;
            default:
                throw std::logic_error("State not implemented");
        }
    }

    private:
    std::mt19937 eng;
    GameState state;
};