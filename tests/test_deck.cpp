#include <catch2/catch_test_macros.hpp>
#include "Defns.hpp"
#include "GameState.hpp"
#include "Deck.hpp"
#include <random>

TEST_CASE("Deal Cards", "[deck]") {
    GameState gs;
    std::mt19937 eng(12345);
    std::array<Card, 10> a;
    std::array<Card, 10> b;

    for (int i = 0; i < 10; i++) {
        Card c = draw_card(gs.deck, eng);
        a[i] = c;
    }

    gs.deck = euchre::constants::deck_reset;
    eng.seed(12345);
    for (int i = 0; i < 10; i++) {
        Card c = draw_card(gs.deck, eng); 
        REQUIRE(c == a[i]);
    }

    // Test that a different seed gives different results.
    eng.seed(12346);
    int matches = 0;
    gs.deck = euchre::constants::deck_reset;

    for (int i = 0; i < 10; i++) {
        Card c = draw_card(gs.deck, eng);
        if (c == a[i]) {
            matches++;
        }
    }
    // This number is random, but we certianly shouldn't get 5 cards that match in the same order.
    REQUIRE(matches < 5);

}