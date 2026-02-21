#include <catch2/catch_test_macros.hpp>
#include "bots/HeuristicBot.hpp"
#include "bots/RandomBot.hpp"
#include "bots/ScriptedBot.hpp"
#include "Env.hpp"
#include "Action.hpp"
#include "Tables.hpp"
#include "bots.hpp"

using euchre::action::ActionId;
using euchre::action::ActionMask;

// Helper: build an Observation with specific hand and trump for bidding tests
static Observation make_bid_obs(Hand hand, Card face_up, uint8_t player, uint8_t dealer, Phase phase) {
    Observation obs{};
    obs.hand = hand;
    obs.face_up_card = face_up;
    obs.player = player;
    obs.dealer = dealer;
    obs.phase = phase;
    obs.trump = Suit::None;
    return obs;
}

// Helper: build an Observation for play trick tests
static Observation make_play_obs(Hand hand, Suit trump, Card lead, uint8_t player,
                                  uint8_t num_played, uint8_t maker_team,
                                  std::array<Card, 4> trick_cards = {}) {
    Observation obs{};
    obs.hand = hand;
    obs.trump = trump;
    obs.lead = lead;
    obs.phase = Phase::PlayTrick;
    obs.player = player;
    obs.num_played = num_played;
    obs.maker_team = maker_team;
    obs.trick_cards = trick_cards;
    return obs;
}

TEST_CASE("HeuristicBot orders up with strong trump hand") {
    HeuristicBot bot{"test"};

    // Hand: Jack of Hearts (right bower), Ace of Hearts, King of Hearts, Ace of Spades, Ace of Clubs
    // Face up: 9 of Hearts -> trump would be Hearts
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RJ}); // Right bower
    hand.give_card(Card{Suit::H, Rank::RA}); // Trump ace
    hand.give_card(Card{Suit::H, Rank::RK}); // Trump king
    hand.give_card(Card{Suit::S, Rank::RA}); // Off ace
    hand.give_card(Card{Suit::C, Rank::RA}); // Off ace

    Card face_up{Suit::H, Rank::R9};
    Observation obs = make_bid_obs(hand, face_up, 1, 0, Phase::BidRound1);
    ActionMask mask = euchre::action::make_mask(euchre::action::Pass, euchre::action::OrderUp);

    ActionId action = bot.select_action(obs, mask);
    REQUIRE(action == euchre::action::OrderUp);
}

TEST_CASE("HeuristicBot passes with weak hand in round 1") {
    HeuristicBot bot{"test"};

    // Hand: 9C, 10C, 9S, 10S, 9D — no hearts, all low
    Hand hand{};
    hand.give_card(Card{Suit::C, Rank::R9});
    hand.give_card(Card{Suit::C, Rank::RT});
    hand.give_card(Card{Suit::S, Rank::R9});
    hand.give_card(Card{Suit::S, Rank::RT});
    hand.give_card(Card{Suit::D, Rank::R9});

    Card face_up{Suit::H, Rank::RK};
    Observation obs = make_bid_obs(hand, face_up, 1, 0, Phase::BidRound1);
    ActionMask mask = euchre::action::make_mask(euchre::action::Pass, euchre::action::OrderUp);

    ActionId action = bot.select_action(obs, mask);
    REQUIRE(action == euchre::action::Pass);
}

TEST_CASE("HeuristicBot calls trump in round 2 with strong suit") {
    HeuristicBot bot{"test"};

    // Hand strong in spades: JS (right), JC (left), AS, KS, 9D
    // Face up was hearts (turned down), so hearts excluded from calls
    Hand hand{};
    hand.give_card(Card{Suit::S, Rank::RJ}); // Right bower if spades trump
    hand.give_card(Card{Suit::C, Rank::RJ}); // Left bower if spades trump
    hand.give_card(Card{Suit::S, Rank::RA});
    hand.give_card(Card{Suit::S, Rank::RK});
    hand.give_card(Card{Suit::D, Rank::R9});

    Observation obs{};
    obs.hand = hand;
    obs.face_up_card = Card{Suit::H, Rank::RK}; // Hearts turned down
    obs.player = 1;
    obs.dealer = 0;
    obs.phase = Phase::BidRound2;
    obs.trump = Suit::None;

    // Mask: Pass + call C, S, D (not H since it was turned down)
    ActionMask mask = euchre::action::make_mask(
        euchre::action::Pass,
        euchre::action::call_trump(Suit::C),
        euchre::action::call_trump(Suit::S),
        euchre::action::call_trump(Suit::D)
    );

    ActionId action = bot.select_action(obs, mask);
    REQUIRE(action == euchre::action::call_trump(Suit::S));
}

TEST_CASE("HeuristicBot discards weakest card") {
    HeuristicBot bot{"test"};

    // Trump is hearts. Hand: JH, AH, KH, 9C, 10C (dealer picked up card already in hand)
    // Should discard 9C or 10C (weakest off-suit)
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RJ}); // Right bower
    hand.give_card(Card{Suit::H, Rank::RA});
    hand.give_card(Card{Suit::H, Rank::RK});
    hand.give_card(Card{Suit::C, Rank::R9});
    hand.give_card(Card{Suit::C, Rank::RT});
    hand.give_card(Card{Suit::C, Rank::RQ}); // 6 cards after pickup

    Observation obs{};
    obs.hand = hand;
    obs.trump = Suit::H;
    obs.phase = Phase::DealerPickupDiscard;
    obs.player = 0;

    // Discard mask: bits 24-47 for cards in hand
    ActionMask mask = static_cast<ActionMask>(hand.value()) << euchre::constants::num_cards;

    ActionId action = bot.select_action(obs, mask);

    // Should discard 9C (weakest card)
    Card discarded{static_cast<uint8_t>(action.v - euchre::constants::num_cards)};
    REQUIRE(discarded == Card(Suit::C, Rank::R9));
}

TEST_CASE("HeuristicBot leads trump when maker with high trump") {
    HeuristicBot bot{"test"};

    // Trump hearts, player 0 is maker team (team 0)
    // Hand: JH (right bower), AD, KS
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RJ}); // Right bower
    hand.give_card(Card{Suit::D, Rank::RA});
    hand.give_card(Card{Suit::S, Rank::RK});

    Observation obs = make_play_obs(hand, Suit::H, Card{euchre::constants::invalid_card},
                                     0, 0, 0); // player 0, num_played 0, maker_team 0

    ActionMask mask = static_cast<ActionMask>(hand.value());
    ActionId action = bot.select_action(obs, mask);

    // Should lead the right bower (highest trump)
    Card played{static_cast<uint8_t>(action.v)};
    REQUIRE(played == Card(Suit::H, Rank::RJ));
}

TEST_CASE("HeuristicBot plays low when partner is winning trick") {
    HeuristicBot bot{"test"};

    // Trump hearts. Player 2 is following. Led suit is spades.
    // Partner (player 0) played AS, opponent (player 1) played 9S.
    // Player 2 has KS, QS — should play lowest (QS) since partner winning.
    Hand hand{};
    hand.give_card(Card{Suit::S, Rank::RK});
    hand.give_card(Card{Suit::S, Rank::RQ});

    std::array<Card, 4> trick_cards{};
    trick_cards[0] = Card{Suit::S, Rank::RA}; // Partner (player 0)
    trick_cards[1] = Card{Suit::S, Rank::R9}; // Opponent (player 1)
    trick_cards[2] = Card{euchre::constants::invalid_card};
    trick_cards[3] = Card{euchre::constants::invalid_card};

    Card lead{Suit::S, Rank::RA};
    Observation obs = make_play_obs(hand, Suit::H, lead, 2, 2, 0, trick_cards);

    // Valid plays: must follow spades
    ActionMask mask = static_cast<ActionMask>(hand.value());
    ActionId action = bot.select_action(obs, mask);

    Card played{static_cast<uint8_t>(action.v)};
    REQUIRE(played == Card(Suit::S, Rank::RQ));
}

TEST_CASE("HeuristicBot goes alone with very strong hand") {
    HeuristicBot bot{"test"};

    // Trump hearts. Hand: JH (right), JD (left), AH, KH, QH — 5 trump
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RJ}); // Right bower
    hand.give_card(Card{Suit::D, Rank::RJ}); // Left bower
    hand.give_card(Card{Suit::H, Rank::RA});
    hand.give_card(Card{Suit::H, Rank::RK});
    hand.give_card(Card{Suit::H, Rank::RQ});

    Observation obs{};
    obs.hand = hand;
    obs.trump = Suit::H;
    obs.phase = Phase::GoAloneDecision;
    obs.player = 1;
    obs.maker_team = 1;

    ActionMask mask = euchre::action::make_mask(euchre::action::GoAloneYes, euchre::action::GoAloneNo);
    ActionId action = bot.select_action(obs, mask);
    REQUIRE(action == euchre::action::GoAloneYes);
}

TEST_CASE("HeuristicBot does not go alone with mediocre hand") {
    HeuristicBot bot{"test"};

    // Trump hearts. Hand: AH, KH, 9D, 10S, QC — only 2 trump, no bowers
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RA});
    hand.give_card(Card{Suit::H, Rank::RK});
    hand.give_card(Card{Suit::D, Rank::R9});
    hand.give_card(Card{Suit::S, Rank::RT});
    hand.give_card(Card{Suit::C, Rank::RQ});

    Observation obs{};
    obs.hand = hand;
    obs.trump = Suit::H;
    obs.phase = Phase::GoAloneDecision;
    obs.player = 1;
    obs.maker_team = 1;

    ActionMask mask = euchre::action::make_mask(euchre::action::GoAloneYes, euchre::action::GoAloneNo);
    ActionId action = bot.select_action(obs, mask);
    REQUIRE(action == euchre::action::GoAloneNo);
}

TEST_CASE("HeuristicBot full game completes without errors") {
    HeuristicBot h0{"H0"}, h1{"H1"}, h2{"H2"}, h3{"H3"};
    std::array<IBot*, 4> players = {&h0, &h1, &h2, &h3};

    // Run multiple games to exercise various code paths
    for (unsigned int seed = 0; seed < 100; seed++) {
        Env env{seed, players};
        int steps = 0;
        while (env.state.status != GameState::GameStatus::GameOver && steps < 10000) {
            env.step_game();
            steps++;
        }
        REQUIRE(env.state.status == GameState::GameStatus::GameOver);
    }
}

TEST_CASE("HeuristicBot beats RandomBot significantly") {
    HeuristicBot h0{"H0"}, h1{"H1"};
    RandomBot r0{"R0"}, r1{"R1"};
    std::array<IBot*, 4> players = {&h0, &r0, &h1, &r1};

    int heuristic_wins = 0;
    constexpr int num_games = 1000;

    for (int g = 0; g < num_games; g++) {
        Env env{static_cast<unsigned int>(g), players};
        int steps = 0;
        while (env.state.status != GameState::GameStatus::GameOver && steps < 10000) {
            env.step_game();
            steps++;
        }
        if (env.state.status == GameState::GameStatus::GameOver && env.state.scores[0] >= 10) {
            heuristic_wins++;
        }
    }

    // Heuristic should win >60% of games
    REQUIRE(heuristic_wins > 600);
}
