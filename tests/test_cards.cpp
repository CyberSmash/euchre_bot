#include <catch2/catch_test_macros.hpp>
#include "Card.hpp"

TEST_CASE("Card encoding round-trips", "[card]") {

    for (uint8_t s = 0; s < 4; ++s) {
        for (uint8_t r = 0; r < 6; ++r) {
            Card c = make_card(Suit(s), Rank(r));
            REQUIRE(uint8_t(c.get_suit()) == s);
            REQUIRE(uint8_t(c.get_rank()) == r);
        }
    }
}

TEST_CASE("Bower Checks", "[card]") {
    Card c = make_card(Suit::S, Rank::RJ);
    REQUIRE(c.is_right_bower(Suit::S));
    REQUIRE_FALSE(c.is_left_bower(Suit::S));

    REQUIRE(c.is_left_bower(Suit::C));
    REQUIRE_FALSE(c.is_left_bower(Suit::D));
}

TEST_CASE("Bower Checks - Hearts/Diamonds pair", "[card]") {
    Card c = make_card(Suit::H, Rank::RJ);
    REQUIRE(c.is_right_bower(Suit::H));
    REQUIRE(c.is_left_bower(Suit::D));
    REQUIRE_FALSE(c.is_left_bower(Suit::H));
    REQUIRE_FALSE(c.is_left_bower(Suit::S));
    REQUIRE_FALSE(c.is_left_bower(Suit::C));

    Card c2 = make_card(Suit::D, Rank::RJ);
    REQUIRE(c2.is_right_bower(Suit::D));
    REQUIRE(c2.is_left_bower(Suit::H));
    REQUIRE_FALSE(c2.is_left_bower(Suit::D));
}