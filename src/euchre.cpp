#include "Card.hpp"
#include "Tables.hpp"
#include <iostream>

void run() {
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
 
    std::cout << "C1 Power (H) " << static_cast<uint32_t>(t.power[Suit::S][Suit::C][c]) << std::endl;
    


}