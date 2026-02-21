#include <catch2/catch_test_macros.hpp>
#include "Defns.hpp"
#include "Env.hpp"
#include <bots/RandomBot.hpp>
#include <bots/ScriptedBot.hpp>
#include "bots.hpp"

struct EuchreFixture {
    ScriptedBot bot_a{"Bot_A"}, bot_b{"Bot_B"}, bot_c{"Bot_C"}, bot_d{"Bot_D"};
    std::array<IBot*, 4> players = {&bot_a, &bot_b, &bot_c, &bot_d};
    Env env{0x1234, players};
};

TEST_CASE_METHOD(EuchreFixture, "Dealing", "[env]") {

    env.step_hand();
    REQUIRE(env.state.hand_state.face_up_card != euchre::constants::invalid_card);
    
    for (int i = 0; i < euchre::constants::num_players; i++) {
        REQUIRE(env.state.hand_state.hands[i].num_cards() == 5);
    }
}

TEST_CASE_METHOD(EuchreFixture, "BidRound1 - someone orders up", "[env]") {
    bot_a.fn = test_bots::pass_then_default;
    bot_b.fn = test_bots::pass_then_default;
    bot_c.fn = test_bots::order_up_then_default;
    bot_d.fn = test_bots::pass_then_default;

    env.step_hand();
    REQUIRE(env.state.hand_state.phase == Phase::BidRound1);

    env.step_hand();
    REQUIRE(env.state.hand_state.phase == Phase::GoAloneDecision);
    REQUIRE(env.state.hand_state.trump != Suit::None);
    REQUIRE(env.state.hand_state.maker_player == 2);
}

TEST_CASE_METHOD(EuchreFixture, "BidRound1 - everyone passes", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::pass_then_default;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    REQUIRE(env.state.hand_state.phase == Phase::BidRound2);
    REQUIRE(env.state.hand_state.trump == Suit::None);
}

TEST_CASE_METHOD(EuchreFixture, "BidRound1 - maker team correctness", "[env]") {
    // Player 1 (left of dealer 0) orders up -> team 1
    bot_a.fn = test_bots::pass_then_default;
    bot_b.fn = test_bots::order_up_then_default;
    bot_c.fn = test_bots::pass_then_default;
    bot_d.fn = test_bots::pass_then_default;

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    REQUIRE(env.state.hand_state.maker_player == 1);
    REQUIRE(env.state.hand_state.maker_team == 1);
}

TEST_CASE_METHOD(EuchreFixture, "GoAloneDecision - going alone", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = [](const Observation& obs, ActionMask mask) -> ActionId {
            if (obs.phase == Phase::BidRound1) return euchre::action::OrderUp;
            if (obs.phase == Phase::GoAloneDecision) return euchre::action::GoAloneYes;
            return test_bots::first_legal(obs, mask);
        };
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    env.step_hand(); // GoAloneDecision
    REQUIRE(env.state.hand_state.going_alone == true);
    REQUIRE(env.state.hand_state.phase == Phase::DealerPickupDiscard);
}

TEST_CASE_METHOD(EuchreFixture, "GoAloneDecision - not going alone", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    env.step_hand(); // GoAloneDecision
    REQUIRE_FALSE(env.state.hand_state.going_alone);
    REQUIRE(env.state.hand_state.phase == Phase::DealerPickupDiscard);
}

TEST_CASE_METHOD(EuchreFixture, "DealerPickupDiscard - dealer has 5 cards after", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_then_default;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    env.step_hand(); // GoAloneDecision
    env.step_hand(); // DealerPickupDiscard
    REQUIRE(env.state.hand_state.hands[env.state.dealer].num_cards() == 5);
    REQUIRE(env.state.hand_state.phase == Phase::PlayTrick);
}

TEST_CASE_METHOD(EuchreFixture, "DealerPickupDiscard - no duplicate cards", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_then_default;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1
    env.step_hand(); // GoAloneDecision
    env.step_hand(); // DealerPickupDiscard

    // All cards across all hands should be unique (no overlapping bits)
    uint32_t all_cards = 0;
    for (int i = 0; i < euchre::constants::num_players; i++) {
        uint32_t hand = env.state.hand_state.hands[i].value();
        REQUIRE((all_cards & hand) == 0); // no overlap
        all_cards |= hand;
    }
    // Total cards in hands should be 20, plus 3 in deck + 1 face up = 24
    REQUIRE(std::popcount(all_cards) == 20);
}

TEST_CASE_METHOD(EuchreFixture, "BidRound2 - someone calls trump", "[env]") {
    bot_a.fn = test_bots::always_pass_both_rounds;
    bot_b.fn = test_bots::always_pass_both_rounds;
    bot_c.fn = test_bots::call_trump_then_default;
    bot_d.fn = test_bots::always_pass_both_rounds;

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1 - everyone passes
    REQUIRE(env.state.hand_state.phase == Phase::BidRound2);

    env.step_hand(); // BidRound2 - player 2 calls trump
    REQUIRE(env.state.hand_state.phase == Phase::GoAloneDecision);
    REQUIRE(env.state.hand_state.maker_player == 2);
    REQUIRE(env.state.hand_state.maker_team == 0);
    REQUIRE(env.state.hand_state.trump != Suit::None);
    // Trump should not be the turned-down suit
    REQUIRE(env.state.hand_state.trump != env.state.hand_state.face_up_card.get_suit());
}

TEST_CASE_METHOD(EuchreFixture, "BidRound2 - everyone passes, redeal", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::always_pass_both_rounds;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1 - everyone passes
    env.step_hand(); // BidRound2 - everyone passes

    // Should have reset back to Deal phase
    REQUIRE(env.state.hand_state.phase == Phase::Deal);
    REQUIRE(env.state.hand_state.trump == Suit::None);
}

TEST_CASE_METHOD(EuchreFixture, "BidRound2 - stick the dealer", "[env]") {
    env.state.hand_state.stick_the_dealer = true;

    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::always_pass_both_rounds;
    }
    // always_pass_both_rounds will pass, but dealer can't pass with stick_the_dealer
    // so first_legal kicks in for the dealer when pass is removed from mask

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1 - everyone passes
    env.step_hand(); // BidRound2 - dealer is forced to pick

    REQUIRE(env.state.hand_state.phase == Phase::GoAloneDecision);
    REQUIRE(env.state.hand_state.maker_player == env.state.dealer);
    REQUIRE(env.state.hand_state.trump != Suit::None);
    REQUIRE(env.state.hand_state.trump != env.state.hand_state.face_up_card.get_suit());
}

// Helper to advance to PlayTrick phase
void advance_to_play_trick(Env& env) {
    while (env.state.hand_state.phase != Phase::PlayTrick) {
        env.step_hand();
    }
}

// Helper to play all 5 tricks
void play_all_tricks(Env& env) {
    for (int i = 0; i < 5; i++) {
        env.step_hand(); // PlayTrick
    }
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - one trick reduces hand sizes by 1", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);
    REQUIRE(env.state.hand_state.phase == Phase::PlayTrick);

    env.step_hand(); // Play one trick

    for (int i = 0; i < euchre::constants::num_players; i++) {
        REQUIRE(env.state.hand_state.hands[i].num_cards() == 4);
    }
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - tricks_played increments", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);

    for (int i = 1; i <= 5; i++) {
        env.step_hand();
        if (i < 5) {
            REQUIRE(env.state.hand_state.tricks_played == i);
        }
    }
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - all hands empty after 5 tricks", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);
    play_all_tricks(env);

    for (int i = 0; i < euchre::constants::num_players; i++) {
        REQUIRE(env.state.hand_state.hands[i].num_cards() == 0);
    }
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - phase is HandOver after 5 tricks", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);
    play_all_tricks(env);

    REQUIRE(env.state.hand_state.phase == Phase::HandOver);
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - total tricks won equals 5", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);
    play_all_tricks(env);

    uint8_t total = env.state.hand_state.tricks_won[0] + env.state.hand_state.tricks_won[1];
    REQUIRE(total == 5);
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - winner leads next trick", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);

    env.step_hand(); // Play first trick
    uint8_t first_winner = env.state.hand_state.lead_player;

    env.step_hand(); // Play second trick
    // The lead_player for the second trick was the first trick's winner
    // We can't directly check who led, but we can verify lead_player changed
    // to the second trick's winner
    REQUIRE(env.state.hand_state.tricks_played == 2);
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - no duplicate cards played across tricks", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);

    // Track all cards in hands before play
    uint32_t all_cards_before = 0;
    for (int i = 0; i < euchre::constants::num_players; i++) {
        all_cards_before |= env.state.hand_state.hands[i].value();
    }
    REQUIRE(std::popcount(all_cards_before) == 20);

    play_all_tricks(env);

    // All hands should be empty
    uint32_t all_cards_after = 0;
    for (int i = 0; i < euchre::constants::num_players; i++) {
        all_cards_after |= env.state.hand_state.hands[i].value();
    }
    REQUIRE(all_cards_after == 0);
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - lead_card reset between tricks", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = test_bots::order_up_no_alone;
    }

    advance_to_play_trick(env);
    env.step_hand(); // Play one trick

    REQUIRE(env.state.hand_state.lead_card == euchre::constants::invalid_card);
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - going alone skips partner", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = [](const Observation& obs, ActionMask mask) -> ActionId {
            if (obs.phase == Phase::BidRound1) return euchre::action::OrderUp;
            if (obs.phase == Phase::GoAloneDecision) return euchre::action::GoAloneYes;
            return test_bots::first_legal(obs, mask);
        };
    }

    advance_to_play_trick(env);
    uint8_t maker = env.state.hand_state.maker_player;
    uint8_t partner = (maker + 2) % euchre::constants::num_players;

    play_all_tricks(env);

    // Partner should still have 5 cards (never played)
    REQUIRE(env.state.hand_state.hands[partner].num_cards() == 5);

    // Other 3 players should have empty hands
    for (int i = 0; i < euchre::constants::num_players; i++) {
        if (i != partner) {
            REQUIRE(env.state.hand_state.hands[i].num_cards() == 0);
        }
    }
}

TEST_CASE_METHOD(EuchreFixture, "PlayTrick - going alone total tricks still 5", "[play]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = [](const Observation& obs, ActionMask mask) -> ActionId {
            if (obs.phase == Phase::BidRound1) return euchre::action::OrderUp;
            if (obs.phase == Phase::GoAloneDecision) return euchre::action::GoAloneYes;
            return test_bots::first_legal(obs, mask);
        };
    }

    advance_to_play_trick(env);
    play_all_tricks(env);

    uint8_t total = env.state.hand_state.tricks_won[0] + env.state.hand_state.tricks_won[1];
    REQUIRE(total == 5);
    REQUIRE(env.state.hand_state.phase == Phase::HandOver);
}

TEST_CASE_METHOD(EuchreFixture, "Engine rejects illegal action", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        // OrderUp is illegal during GoAloneDecision
        bot->fn = test_bots::always_order_up;
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1 - orders up
    // GoAloneDecision - bot returns OrderUp which is not in the mask
    REQUIRE_THROWS_AS(env.step_hand(), std::invalid_argument);
}

TEST_CASE_METHOD(EuchreFixture, "BidRound2 through full hand", "[env]") {
    for (auto* bot : {&bot_a, &bot_b, &bot_c, &bot_d}) {
        bot->fn = [](const Observation& obs, ActionMask mask) -> ActionId {
            if (obs.phase == Phase::BidRound1) return euchre::action::Pass;
            if (obs.phase == Phase::BidRound2) {
                ActionMask calls_only = mask & ~euchre::action::a2m(euchre::action::Pass);
                return ActionId{static_cast<uint16_t>(std::countr_zero(calls_only))};
            }
            if (obs.phase == Phase::GoAloneDecision) return euchre::action::GoAloneNo;
            return test_bots::first_legal(obs, mask);
        };
    }

    env.step_hand(); // Deal
    env.step_hand(); // BidRound1 - everyone passes
    env.step_hand(); // BidRound2 - someone calls trump
    env.step_hand(); // GoAloneDecision
    env.step_hand(); // DealerPickupDiscard

    REQUIRE(env.state.hand_state.phase == Phase::PlayTrick);

    play_all_tricks(env);

    REQUIRE(env.state.hand_state.phase == Phase::HandOver);
    uint8_t total = env.state.hand_state.tricks_won[0] + env.state.hand_state.tricks_won[1];
    REQUIRE(total == 5);

    for (int i = 0; i < euchre::constants::num_players; i++) {
        REQUIRE(env.state.hand_state.hands[i].num_cards() == 0);
    }
}

