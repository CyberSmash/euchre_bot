#include <catch2/catch_test_macros.hpp>
#include "Hand.hpp"
#include "GameState.hpp"
#include "Card.hpp"

TEST_CASE("Valid play (simple)", "[hand]") {
    Hand hand = 0;

    std::array<Card, 5> cards = {
        Card{Suit::C, Rank::R9},
        Card{Suit::S, Rank::RQ},
        Card{Suit::D, Rank::RJ},
        Card{Suit::S, Rank::RT},
        Card{Suit::H, Rank::RK},
    };

    for (const auto& c : cards) {
        hand.give_card(c);
    }

    REQUIRE(!hand.empty());

    Hand valid_hand = hand.get_valid_hand({Suit::D, Rank::RK}, Suit::S);
    REQUIRE(hand.value() != valid_hand.value());
    REQUIRE(valid_hand.hand_has(Card{Suit::D, Rank::RJ}));
}

TEST_CASE("Valid plays (void)", "[hand]") {
        Hand hand = 0;

    std::array<Card, 5> cards = {
        Card{Suit::C, Rank::R9},
        Card{Suit::S, Rank::RQ},
        Card{Suit::H, Rank::RJ},
        Card{Suit::S, Rank::RT},
        Card{Suit::H, Rank::RK},
    };

    for (const auto& c : cards) {
        hand.give_card(c);
    }

    Card lead_card = Card {Suit::D, Rank::RK};
    Hand valid_mask = hand.get_valid_hand(lead_card, Suit::C);
    REQUIRE(!valid_mask.empty());
    REQUIRE((hand.value() & valid_mask.value()) == hand.value());
}

TEST_CASE("Valid plays (left bower)", "[hand]") {
        Hand hand = 0;

    std::array<Card, 5> cards = {
        Card{Suit::C, Rank::R9},
        Card{Suit::S, Rank::RQ},
        Card{Suit::H, Rank::RJ},
        Card{Suit::S, Rank::RT},
        Card{Suit::H, Rank::RK},
    };

    for (const auto& c : cards) {
        hand.give_card(c);
    }

    Card lead_card = Card {Suit::D, Rank::RK};
    Hand valid_cards = hand.get_valid_hand(lead_card, Suit::D);

    REQUIRE(!valid_cards.empty());
    REQUIRE(valid_cards.hand_has({Suit::H, Rank::RJ}));
    REQUIRE(valid_cards.num_cards(valid_cards) == 1);
}


 