#include <catch2/catch_test_macros.hpp>
#include "Action.hpp"

using namespace euchre::action;

TEST_CASE("a2m produces correct bitmask", "[action]") {
    REQUIRE(a2m(ActionId{0}) == 1ULL);
    REQUIRE(a2m(ActionId{1}) == 2ULL);
    REQUIRE(a2m(Pass) == (1ULL << 48));
    REQUIRE(a2m(OrderUp) == (1ULL << 49));
    REQUIRE(a2m(GoAloneYes) == (1ULL << 54));
    REQUIRE(a2m(GoAloneNo) == (1ULL << 55));
}

TEST_CASE("make_mask combines multiple actions", "[action]") {
    ActionMask mask = make_mask(Pass, OrderUp);
    REQUIRE(mask == ((1ULL << 48) | (1ULL << 49)));

    ActionMask mask2 = make_mask(GoAloneYes, GoAloneNo);
    REQUIRE(mask2 == ((1ULL << 54) | (1ULL << 55)));

    // Single action
    ActionMask mask3 = make_mask(Pass);
    REQUIRE(mask3 == (1ULL << 48));
}

TEST_CASE("play encodes card to action", "[action]") {
    for (uint8_t i = 0; i < 24; i++) {
        Card c{i};
        ActionId a = play(c);
        REQUIRE(a.v == i);
        REQUIRE(is_play(a));
        REQUIRE_FALSE(is_discard(a));
    }
}

TEST_CASE("discard encodes card to action", "[action]") {
    for (uint8_t i = 0; i < 24; i++) {
        Card c{i};
        ActionId a = discard(c);
        REQUIRE(a.v == i + euchre::constants::num_cards);
        REQUIRE(is_discard(a));
        REQUIRE_FALSE(is_play(a));
    }
}

TEST_CASE("call_trump encodes suit to action", "[action]") {
    REQUIRE(call_trump(Suit::C).v == 50);
    REQUIRE(call_trump(Suit::H).v == 51);
    REQUIRE(call_trump(Suit::S).v == 52);
    REQUIRE(call_trump(Suit::D).v == 53);

    for (uint8_t s = 0; s < 4; s++) {
        ActionId a = call_trump(Suit(s));
        REQUIRE(is_call_trump(a));
        REQUIRE_FALSE(is_play(a));
        REQUIRE_FALSE(is_discard(a));
    }
}

TEST_CASE("decode_action round-trips play actions", "[action]") {
    for (uint8_t i = 0; i < 24; i++) {
        Card c{i};
        ActionId a = play(c);
        DecodedAction d = decode_action(a);
        REQUIRE(d.kind == ActionKind::PlayCard);
        REQUIRE(d.card == c);
        REQUIRE(d.suit == c.get_suit());
    }
}

TEST_CASE("decode_action round-trips discard actions", "[action]") {
    for (uint8_t i = 0; i < 24; i++) {
        Card c{i};
        ActionId a = discard(c);
        DecodedAction d = decode_action(a);
        REQUIRE(d.kind == ActionKind::DiscardCard);
        REQUIRE(d.card == c);
        REQUIRE(d.suit == c.get_suit());
    }
}

TEST_CASE("decode_action handles fixed actions", "[action]") {
    REQUIRE(decode_action(Pass).kind == ActionKind::Pass);
    REQUIRE(decode_action(OrderUp).kind == ActionKind::OrderUp);
    REQUIRE(decode_action(GoAloneYes).kind == ActionKind::GoAloneYes);
    REQUIRE(decode_action(GoAloneNo).kind == ActionKind::GoAloneNo);
    REQUIRE(decode_action(InvalidAction).kind == ActionKind::InvalidAction);
}

TEST_CASE("decode_action round-trips call trump actions", "[action]") {
    for (uint8_t s = 0; s < 4; s++) {
        ActionId a = call_trump(Suit(s));
        DecodedAction d = decode_action(a);
        REQUIRE(d.kind == ActionKind::CallTrump);
        REQUIRE(d.suit == Suit(s));
    }
}

TEST_CASE("is_play boundary checks", "[action]") {
    REQUIRE(is_play(ActionId{0}));
    REQUIRE(is_play(ActionId{23}));
    REQUIRE_FALSE(is_play(ActionId{24}));
    REQUIRE_FALSE(is_play(Pass));
}

TEST_CASE("is_discard boundary checks", "[action]") {
    REQUIRE(is_discard(ActionId{24}));
    REQUIRE(is_discard(ActionId{47}));
    REQUIRE_FALSE(is_discard(ActionId{23}));
    REQUIRE_FALSE(is_discard(ActionId{48}));
}

TEST_CASE("is_call_trump boundary checks", "[action]") {
    REQUIRE(is_call_trump(ActionId{50}));
    REQUIRE(is_call_trump(ActionId{53}));
    REQUIRE_FALSE(is_call_trump(ActionId{49}));
    REQUIRE_FALSE(is_call_trump(ActionId{54}));
}
