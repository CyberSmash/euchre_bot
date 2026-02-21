#include <catch2/catch_test_macros.hpp>
#include "Defns.hpp"
#include "Env.hpp"
#include <bots/ScriptedBot.hpp>
#include "bots.hpp"

struct GameFixture {
    ScriptedBot bot_a{"Bot_A"}, bot_b{"Bot_B"}, bot_c{"Bot_C"}, bot_d{"Bot_D"};
    std::array<IBot*, 4> players = {&bot_a, &bot_b, &bot_c, &bot_d};
    Env env{0x1234, players};
};

// -- Scoring tests (call hand_over directly with pre-configured state) --

TEST_CASE_METHOD(GameFixture, "Scoring - maker wins 3 tricks = 1 point", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.tricks_won[1] = 2;
    env.state.hand_state.going_alone = false;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 1);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - maker wins 4 tricks = 1 point", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 4;
    env.state.hand_state.tricks_won[1] = 1;
    env.state.hand_state.going_alone = false;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 1);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - maker sweep = 2 points", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 5;
    env.state.hand_state.tricks_won[1] = 0;
    env.state.hand_state.going_alone = false;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 2);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - euchre = 2 points for defenders", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 2;
    env.state.hand_state.tricks_won[1] = 3;
    env.state.hand_state.going_alone = false;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 0);
    REQUIRE(env.state.scores[1] == 2);
}

TEST_CASE_METHOD(GameFixture, "Scoring - euchre with 0 tricks = 2 points for defenders", "[scoring]") {
    env.state.hand_state.maker_team = 1;
    env.state.hand_state.tricks_won[0] = 5;
    env.state.hand_state.tricks_won[1] = 0;
    env.state.hand_state.going_alone = false;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 2);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - going alone sweep = 4 points", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 5;
    env.state.hand_state.tricks_won[1] = 0;
    env.state.hand_state.going_alone = true;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 4);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - going alone wins 3 = 1 point", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.tricks_won[1] = 2;
    env.state.hand_state.going_alone = true;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 1);
    REQUIRE(env.state.scores[1] == 0);
}

TEST_CASE_METHOD(GameFixture, "Scoring - going alone euchred = 2 points for defenders", "[scoring]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 1;
    env.state.hand_state.tricks_won[1] = 4;
    env.state.hand_state.going_alone = true;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.scores[0] == 0);
    REQUIRE(env.state.scores[1] == 2);
}

// -- Hand-over mechanics --

TEST_CASE_METHOD(GameFixture, "HandOver - dealer rotates", "[game]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.phase = Phase::HandOver;

    REQUIRE(env.state.dealer == 0);
    env.hand_over();
    REQUIRE(env.state.dealer == 1);

    // Set up another hand over
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.phase = Phase::HandOver;
    env.hand_over();
    REQUIRE(env.state.dealer == 2);
}

TEST_CASE_METHOD(GameFixture, "HandOver - dealer wraps around", "[game]") {
    env.state.dealer = 3;
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();
    REQUIRE(env.state.dealer == 0);
}

TEST_CASE_METHOD(GameFixture, "HandOver - hand state resets", "[game]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.trump = Suit::H;
    env.state.hand_state.going_alone = true;
    env.state.hand_state.maker_player = 2;
    env.state.hand_state.phase = Phase::HandOver;

    env.hand_over();

    REQUIRE(env.state.hand_state.phase == Phase::Deal);
    REQUIRE(env.state.hand_state.trump == Suit::None);
    REQUIRE(env.state.hand_state.going_alone == false);
    REQUIRE(env.state.hand_state.tricks_won[0] == 0);
    REQUIRE(env.state.hand_state.tricks_won[1] == 0);
    REQUIRE(env.state.hand_state.tricks_played == 0);
}

TEST_CASE_METHOD(GameFixture, "HandOver - scores persist across hands", "[game]") {
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 5;
    env.state.hand_state.phase = Phase::HandOver;
    env.hand_over();
    REQUIRE(env.state.scores[0] == 2);

    // Second hand - team 1 gets euchred
    env.state.hand_state.maker_team = 1;
    env.state.hand_state.tricks_won[0] = 4;
    env.state.hand_state.tricks_won[1] = 1;
    env.state.hand_state.phase = Phase::HandOver;
    env.hand_over();

    // Team 0 should have 2 + 2 = 4
    REQUIRE(env.state.scores[0] == 4);
    REQUIRE(env.state.scores[1] == 0);
}

// -- Game-over detection --

TEST_CASE_METHOD(GameFixture, "GameOver - detected when team reaches 10", "[game]") {
    env.state.scores[0] = 9;
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.phase = Phase::HandOver;

    env.step_game();

    REQUIRE(env.state.scores[0] == 10);
    REQUIRE(env.state.status == GameState::GameStatus::GameOver);
}

TEST_CASE_METHOD(GameFixture, "GameOver - not triggered below 10", "[game]") {
    env.state.scores[0] = 8;
    env.state.hand_state.maker_team = 0;
    env.state.hand_state.tricks_won[0] = 3;
    env.state.hand_state.phase = Phase::HandOver;

    env.step_game();

    REQUIRE(env.state.scores[0] == 9);
    REQUIRE(env.state.status == GameState::GameStatus::InProgress);
}

TEST_CASE_METHOD(GameFixture, "GameOver - can exceed 10 with sweep", "[game]") {
    env.state.scores[1] = 9;
    env.state.hand_state.maker_team = 1;
    env.state.hand_state.tricks_won[1] = 5;
    env.state.hand_state.going_alone = true;
    env.state.hand_state.phase = Phase::HandOver;

    env.step_game();

    REQUIRE(env.state.scores[1] == 13);
    REQUIRE(env.state.status == GameState::GameStatus::GameOver);
}

// -- Full game integration --

TEST_CASE_METHOD(GameFixture, "Full game terminates", "[game]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    int max_steps = 10000;
    int steps = 0;
    while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        env.step_game();
        steps++;
    }

    REQUIRE(env.state.status == GameState::GameStatus::GameOver);
    REQUIRE((env.state.scores[0] >= 10 || env.state.scores[1] >= 10));
}

TEST_CASE_METHOD(GameFixture, "Full game - scores only increase", "[game]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    uint8_t prev_scores[2] = {0, 0};
    int max_steps = 10000;
    int steps = 0;

    while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        env.step_game();
        REQUIRE(env.state.scores[0] >= prev_scores[0]);
        REQUIRE(env.state.scores[1] >= prev_scores[1]);
        prev_scores[0] = env.state.scores[0];
        prev_scores[1] = env.state.scores[1];
        steps++;
    }
}

TEST_CASE_METHOD(GameFixture, "Full game - dealer rotates correctly", "[game]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    int max_steps = 10000;
    int steps = 0;
    uint8_t expected_dealer = 0;

    while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        if (env.state.hand_state.phase == Phase::Deal) {
            REQUIRE(env.state.dealer == expected_dealer);
        }
        if (env.state.hand_state.phase == Phase::HandOver) {
            expected_dealer = (expected_dealer + 1) % euchre::constants::num_players;
        }
        env.step_game();
        steps++;
    }
}

TEST_CASE_METHOD(GameFixture, "Full game - point values are valid", "[game]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    uint8_t prev_total = 0;
    int max_steps = 10000;
    int steps = 0;

    while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        env.step_game();
        uint8_t total = env.state.scores[0] + env.state.scores[1];
        if (total > prev_total) {
            uint8_t gained = total - prev_total;
            // Points gained per hand must be 1, 2, or 4
            REQUIRE((gained == 1 || gained == 2 || gained == 4));
            prev_total = total;
        }
        steps++;
    }
}

TEST_CASE_METHOD(GameFixture, "Full game - losing team under 10", "[game]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    int max_steps = 10000;
    int steps = 0;
    while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        env.step_game();
        steps++;
    }

    REQUIRE(env.state.status == GameState::GameStatus::GameOver);
    // Exactly one team should be >= 10
    bool team0_won = env.state.scores[0] >= 10;
    bool team1_won = env.state.scores[1] >= 10;
    REQUIRE((team0_won || team1_won));
    // Losing team must be under 10
    if (team0_won) {
        REQUIRE(env.state.scores[1] < 10);
    } else {
        REQUIRE(env.state.scores[0] < 10);
    }
}

TEST_CASE_METHOD(GameFixture, "Full game with different seed", "[game]") {
    Env env2{0xDEAD, players};
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    int max_steps = 10000;
    int steps = 0;
    while (env2.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
        env2.step_game();
        steps++;
    }

    REQUIRE(env2.state.status == GameState::GameStatus::GameOver);
    REQUIRE((env2.state.scores[0] >= 10 || env2.state.scores[1] >= 10));
}
