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

TEST_CASE("remove_card", "[hand]") {
    Hand hand = 0;
    Card c1{Suit::C, Rank::R9};
    Card c2{Suit::H, Rank::RK};

    hand.give_card(c1);
    hand.give_card(c2);
    REQUIRE(hand.num_cards() == 2);

    hand.remove_card(c1);
    REQUIRE(hand.num_cards() == 1);
    REQUIRE_FALSE(hand.hand_has(c1));
    REQUIRE(hand.hand_has(c2));
}

TEST_CASE("hand_has returns false for missing card", "[hand]") {
    Hand hand = 0;
    hand.give_card(Card{Suit::C, Rank::R9});

    REQUIRE_FALSE(hand.hand_has(Card{Suit::H, Rank::RK}));
    REQUIRE_FALSE(hand.hand_has(Card{Suit::C, Rank::RT}));
    REQUIRE_FALSE(hand.hand_has(Card{Suit::D, Rank::RA}));
}

TEST_CASE("Left bower follows trump suit", "[hand]") {
    // When trump is led and you only have the left bower, it must be played
    Hand hand = 0;
    hand.give_card(Card{Suit::C, Rank::RJ}); // Left bower when Spades is trump
    hand.give_card(Card{Suit::H, Rank::RK});
    hand.give_card(Card{Suit::D, Rank::RA});

    Card led = make_card(Suit::S, Rank::R9); // Spades led
    Suit trump = Suit::S;
    Hand valid = hand.get_valid_hand(led, trump);

    // Only the left bower (Jack of Clubs) should be playable
    REQUIRE(valid.hand_has(Card{Suit::C, Rank::RJ}));
    REQUIRE(valid.num_cards() == 1);
}

TEST_CASE("Left bower not treated as its natural suit", "[hand]") {
    // When Clubs is led and Spades is trump, Jack of Clubs is trump, not Clubs
    Hand hand = 0;
    hand.give_card(Card{Suit::C, Rank::RJ}); // Left bower (trump, not clubs)
    hand.give_card(Card{Suit::H, Rank::RK});
    hand.give_card(Card{Suit::D, Rank::RA});

    Card led = make_card(Suit::C, Rank::R9); // Clubs led
    Suit trump = Suit::S;
    Hand valid = hand.get_valid_hand(led, trump);

    // Void in clubs (left bower is trump), so full hand is valid
    REQUIRE(valid.num_cards() == 3);
}
