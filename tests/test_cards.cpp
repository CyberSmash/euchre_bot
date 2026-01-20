#include <catch2/catch_test_macros.hpp>
#include "Card.hpp"

TEST_CASE("Card encoding round-trips", "[card]") {

    for (uint8_t s = 0; s < 4; ++s) {
        for (uint8_t r = 0; r < 6; ++r) {
            Card c = make_card(Suit(s), Rank(r));
            REQUIRE(uint8_t(get_suit(c)) == s);
            REQUIRE(uint8_t(get_rank(c)) == r);
        }
    }
}

TEST_CASE("Bower Checks", "[card]") {
    Card c = make_card(Suit::S, Rank::RJ);
    REQUIRE(is_right_bower(c, Suit::S));
    REQUIRE_FALSE(is_left_bower(c, Suit::S));

    REQUIRE(is_left_bower(c, Suit::C));
    REQUIRE_FALSE(is_left_bower(c, Suit::D));
}