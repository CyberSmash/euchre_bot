#include <iostream>
#include <cstdint>
#include <random>
#include "Card.hpp"
#include "Tables.hpp"
#include "GameState.hpp"
#include "Deck.hpp"
#include "Action.hpp"
#include <bit>
#include "bots/RandomBot.hpp"
#include "Hand.hpp"

#define SHOW_FULL_CARD_NAME 1

int main() {
    euchre::action::ActionId action (23);
    GameState gs;
    euchre::tables::Tables t = euchre::tables::tables();

    Card c = make_card(Suit::H, Rank::RK);
    std::cout << "Card: " << c << std::endl;

    Suit eff_suit = t.eff_suit_tbl[Suit::D][c];
    Card c2 = make_card(Suit::S, Rank::RJ);

    std::cout << "Effective Suit: " << eff_suit << std::endl;

    std::cout << "Card 2: " << c2 << std::endl;
    std::cout << "Eff Suit: " << t.eff_suit_tbl[Suit::D][c2] << std::endl;

    Card c3 = make_card(Suit::H, Rank::RJ);
    std::cout << "Card 3: " << c3 << std::endl;
    std::cout << "Eff Suit: " << t.eff_suit_tbl[Suit::D][c3] << std::endl;
 
    std::cout << "C1 Power (H) " << +t.power[Suit::S][Suit::C][c] << std::endl;
    std::mt19937 eng(12345);
    for (size_t i = 0; i < 20; i++) {
        c = draw_card(gs.deck, eng);
        std::cout << "Draw[" << i << "]: " << c << " to player #" << (i % euchre::constants::num_players) + 1 << std::endl;
        //give_card(c, gs.hand_state.hands[i % euchre::constants::num_players]);
        gs.hand_state.give_card_to(c, i % euchre::constants::num_players);
        
    }

    unsigned int idx = std::countr_zero<unsigned int>(2);
    std::cout << idx << std::endl;
    
    for (uint32_t i = 0; i < euchre::constants::num_players; i++ ) {
        //gs.hand_state.show_hand(i);
        //show_hand(gs.hand_state.hands[i]);
        gs.hand_state.hands[i].show_hand();
    }
    


    return 0;

    
}