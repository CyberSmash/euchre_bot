#include <catch2/catch_test_macros.hpp>
#include "bots/MinBot.hpp"
#include "bots/MaxBot.hpp"
#include "bots/RandomBot.hpp"
#include "bots/BotUtils.hpp"
#include "Env.hpp"
#include "Action.hpp"
#include "Tables.hpp"

using euchre::action::ActionId;
using euchre::action::ActionMask;

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

TEST_CASE("MinBot plays lowest-power legal card when following") {
    MinBot bot{"test"};
    auto& t = euchre::tables::tables();

    // Trump hearts. Following spades lead. Hand: AS, 9S, KH
    Hand hand{};
    hand.give_card(Card{Suit::S, Rank::RA});
    hand.give_card(Card{Suit::S, Rank::R9});
    hand.give_card(Card{Suit::H, Rank::RK});

    Card lead{Suit::S, Rank::RK};
    Observation obs = make_play_obs(hand, Suit::H, lead, 1, 1, 0);

    // Must follow spades: AS and 9S are valid
    Hand valid{};
    valid.give_card(Card{Suit::S, Rank::RA});
    valid.give_card(Card{Suit::S, Rank::R9});
    ActionMask mask = static_cast<ActionMask>(valid.value());

    ActionId action = bot.select_action(obs, mask);
    Card played{static_cast<uint8_t>(action.v)};

    // 9S has lower power than AS in spades led context
    Suit led_suit = t.eff_suit_tbl[Suit::H][lead];
    uint8_t power_9s = t.power[Suit::H][led_suit][Card{Suit::S, Rank::R9}];
    uint8_t power_as = t.power[Suit::H][led_suit][Card{Suit::S, Rank::RA}];
    REQUIRE(power_9s < power_as);
    REQUIRE(played == Card(Suit::S, Rank::R9));
}

TEST_CASE("MaxBot plays highest-power legal card when following") {
    MaxBot bot{"test"};
    auto& t = euchre::tables::tables();

    // Trump hearts. Following spades lead. Hand: AS, 9S, KH
    Hand hand{};
    hand.give_card(Card{Suit::S, Rank::RA});
    hand.give_card(Card{Suit::S, Rank::R9});
    hand.give_card(Card{Suit::H, Rank::RK});

    Card lead{Suit::S, Rank::RK};
    Observation obs = make_play_obs(hand, Suit::H, lead, 1, 1, 0);

    // Must follow spades: AS and 9S are valid
    Hand valid{};
    valid.give_card(Card{Suit::S, Rank::RA});
    valid.give_card(Card{Suit::S, Rank::R9});
    ActionMask mask = static_cast<ActionMask>(valid.value());

    ActionId action = bot.select_action(obs, mask);
    Card played{static_cast<uint8_t>(action.v)};

    Suit led_suit = t.eff_suit_tbl[Suit::H][lead];
    uint8_t power_9s = t.power[Suit::H][led_suit][Card{Suit::S, Rank::R9}];
    uint8_t power_as = t.power[Suit::H][led_suit][Card{Suit::S, Rank::RA}];
    REQUIRE(power_as > power_9s);
    REQUIRE(played == Card(Suit::S, Rank::RA));
}

TEST_CASE("MinBot plays lowest card when leading") {
    MinBot bot{"test"};

    // Trump hearts. Leading. Hand: AH, 9C, KS
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RA});
    hand.give_card(Card{Suit::C, Rank::R9});
    hand.give_card(Card{Suit::S, Rank::RK});

    Observation obs = make_play_obs(hand, Suit::H, Card{euchre::constants::invalid_card},
                                     0, 0, 0);
    ActionMask mask = static_cast<ActionMask>(hand.value());

    ActionId action = bot.select_action(obs, mask);
    Card played{static_cast<uint8_t>(action.v)};

    // 9C should be lowest power when trump is led_suit (leading uses trump)
    REQUIRE(played == Card(Suit::C, Rank::R9));
}

TEST_CASE("MaxBot plays highest card when leading") {
    MaxBot bot{"test"};

    // Trump hearts. Leading. Hand: AH, 9C, KS
    Hand hand{};
    hand.give_card(Card{Suit::H, Rank::RA});
    hand.give_card(Card{Suit::C, Rank::R9});
    hand.give_card(Card{Suit::S, Rank::RK});

    Observation obs = make_play_obs(hand, Suit::H, Card{euchre::constants::invalid_card},
                                     0, 0, 0);
    ActionMask mask = static_cast<ActionMask>(hand.value());

    ActionId action = bot.select_action(obs, mask);
    Card played{static_cast<uint8_t>(action.v)};

    // AH (trump ace) should be highest power
    REQUIRE(played == Card(Suit::H, Rank::RA));
}

TEST_CASE("MinBot full game completes without errors") {
    MinBot m0{"M0"}, m1{"M1"}, m2{"M2"}, m3{"M3"};
    std::array<IBot*, 4> players = {&m0, &m1, &m2, &m3};

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

TEST_CASE("MaxBot full game completes without errors") {
    MaxBot m0{"M0"}, m1{"M1"}, m2{"M2"}, m3{"M3"};
    std::array<IBot*, 4> players = {&m0, &m1, &m2, &m3};

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

TEST_CASE("MaxBot beats RandomBot") {
    MaxBot m0{"Max0"}, m1{"Max1"};
    RandomBot r0{"R0"}, r1{"R1"};
    std::array<IBot*, 4> players = {&m0, &r0, &m1, &r1};

    int max_wins = 0;
    constexpr int num_games = 1000;

    for (int g = 0; g < num_games; g++) {
        Env env{static_cast<unsigned int>(g), players};
        int steps = 0;
        while (env.state.status != GameState::GameStatus::GameOver && steps < 10000) {
            env.step_game();
            steps++;
        }
        if (env.state.status == GameState::GameStatus::GameOver && env.state.scores[0] >= 10) {
            max_wins++;
        }
    }

    // MaxBot should win >50% against random
    REQUIRE(max_wins > 500);
}
