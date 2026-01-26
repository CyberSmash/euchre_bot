#include <catch2/catch_test_macros.hpp>
#include "Tables.hpp"

TEST_CASE("Effective Suit", "[tables]") {
    euchre::tables::Tables t = euchre::tables::tables();
    Suit trump = Suit::C;

    Card c = make_card(Suit::S, Rank::RJ);

    Suit eff_s = t.eff_suit_tbl[trump][c];
    REQUIRE(eff_s == trump);

    REQUIRE_FALSE(eff_s == Suit::S);
   
    
    c = make_card(Suit::S, Rank::RK);
    eff_s = t.eff_suit_tbl[trump][c];
    REQUIRE_FALSE(eff_s == trump);
    REQUIRE(eff_s == Suit::S);

}

TEST_CASE("Card Power", "[tables]") {
    euchre::tables::Tables t = euchre::tables::tables();
    Suit led = Suit::D;
    Suit trump = Suit::S;
    Card c1 = make_card(Suit::S, Rank::R9);
    Card c2 = make_card(Suit::D, Rank::RJ);
    uint8_t c1_power = t.power[trump][led][c1];
    uint8_t c2_power = t.power[trump][led][c2];

    REQUIRE(c1_power > c2_power);
    
    // Check that the right bower is being respected in the simple case.
    c1 = make_card(Suit::S, Rank::R9);
    c2 = make_card(Suit::S, Rank::RJ);
    c1_power = t.power[trump][led][c1];
    c2_power = t.power[trump][led][c2];

    REQUIRE(c1_power < c2_power);

    // Check that the right bower is being respected over the King of the same trump suit.
    c1 = make_card(Suit::S, Rank::RK);
    c2 = make_card(Suit::S, Rank::RJ);
    c1_power = t.power[trump][led][c1];
    c2_power = t.power[trump][led][c2];

    REQUIRE(c1_power < c2_power);

    // check that led card doesnt follow bower rules if no suit is trump.
    c1 = make_card(Suit::D, Rank::RK);
    c2 = make_card(Suit::D, Rank::RJ);
    c1_power = t.power[trump][led][c1];
    c2_power = t.power[trump][led][c2];

    REQUIRE(c1_power > c2_power);

    // Test that off suit are always less than the led suit
    c1 = make_card(Suit::H, Rank::RK);
    c2 = make_card(Suit::D, Rank::RJ);
    c1_power = t.power[trump][led][c1];
    c2_power = t.power[trump][led][c2];

    REQUIRE(c1_power < c2_power);
}
