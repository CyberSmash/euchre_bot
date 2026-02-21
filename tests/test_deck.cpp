#include <catch2/catch_test_macros.hpp>
#include "Defns.hpp"
#include "GameState.hpp"
#include "Deck.hpp"
#include <random>

TEST_CASE("Deal Cards", "[deck]") {
    GameState gs;
    std::mt19937 eng(12345);
    std::array<Card, 10> a;
    gs.hand_state.deck = euchre::constants::deck_reset;
    for (std::size_t i = 0; i < 10; i++) {
        Card c = draw_card(gs.hand_state.deck, eng);
        a[i] = c;
        REQUIRE_FALSE(a[i] == euchre::constants::invalid_card);
    }

    gs.hand_state.deck = euchre::constants::deck_reset;
    eng.seed(12345);
    for (std::size_t i = 0; i < 10; i++) {
        Card c = draw_card(gs.hand_state.deck, eng); 
        REQUIRE(c == a[i]);
    }

    // Test that a different seed gives different results.
    eng.seed(12346);
    int matches = 0;
    gs.hand_state.deck = euchre::constants::deck_reset;

    for (size_t i = 0; i < 10; i++) {
        Card c = draw_card(gs.hand_state.deck, eng);
        if (c == a[i]) {
            matches++;
        }
    }
    // This number is random, but we certianly shouldn't get 5 cards that match in the same order.
    REQUIRE(matches < 5);

}

TEST_CASE("Deal No Duplicates", "[deck]") {
    GameState gs;
    std::mt19937 eng(12345);

    for (int i = 0; i < euchre::constants::num_cards; i++) {
        Card c = draw_card(gs.hand_state.deck, eng);
        gs.hand_state.give_card_to(c, i % euchre::constants::num_players);
    }
    uint32_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += gs.hand_state.hands[i].value();
    }

    uint32_t match = gs.hand_state.hands[0].value() | gs.hand_state.hands[1].value() | gs.hand_state.hands[2].value() | gs.hand_state.hands[3].value();
    REQUIRE(match == sum);
}