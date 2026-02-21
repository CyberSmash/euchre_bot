#include "bots/HeuristicBot.hpp"
#include "bots/BotUtils.hpp"
#include "Tables.hpp"
#include <bit>

using euchre::action::ActionId;
using euchre::action::ActionMask;
using bot_utils::HandStrength;
using bot_utils::evaluate_hand;
using bot_utils::lowest_card;
using bot_utils::highest_card;
using bot_utils::cheapest_winner;

ActionId HeuristicBot::bid_phase_1_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    Suit potential_trump = obs.face_up_card.get_suit();
    HandStrength hs = evaluate_hand(obs.hand, potential_trump);

    // Positional bonus: if we or partner are dealer, the pickup helps
    bool dealer_is_partner = (obs.dealer % 2) == (obs.player % 2);
    int effective_score = hs.score + (dealer_is_partner ? 1 : 0);

    // Order up with effective score >= 4
    if (effective_score >= 4) {
        return euchre::action::OrderUp;
    }

    return euchre::action::Pass;
}

ActionId HeuristicBot::bid_phase_2_action(const Observation& obs, ActionMask action_mask) {
    int best_score = 0;
    Suit best_suit = Suit::None;

    for (uint8_t s = 0; s < 4; s++) {
        Suit suit = Suit(s);
        ActionId call = euchre::action::call_trump(suit);
        if ((euchre::action::a2m(call) & action_mask) == 0) continue;

        HandStrength hs = evaluate_hand(obs.hand, suit);
        if (hs.score > best_score) {
            best_score = hs.score;
            best_suit = suit;
        }
    }

    // Call if strength meets threshold
    if (best_score >= 4 && best_suit != Suit::None) {
        return euchre::action::call_trump(best_suit);
    }

    // If pass is available, pass
    if (action_mask & euchre::action::a2m(euchre::action::Pass)) {
        return euchre::action::Pass;
    }

    // Stick-the-dealer: must call best available
    if (best_suit != Suit::None) {
        return euchre::action::call_trump(best_suit);
    }

    // Fallback: pick first legal call
    ActionMask calls_only = action_mask & ~euchre::action::a2m(euchre::action::Pass);
    return ActionId{static_cast<uint16_t>(std::countr_zero(calls_only))};
}

ActionId HeuristicBot::go_alone_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    HandStrength hs = evaluate_hand(obs.hand, obs.trump);

    // Go alone with very strong hands
    if (hs.trump_count >= 4 && hs.has_right) {
        return euchre::action::GoAloneYes;
    }
    if (hs.has_right && hs.has_left && hs.trump_count >= 3 && hs.off_aces >= 1) {
        return euchre::action::GoAloneYes;
    }

    return euchre::action::GoAloneNo;
}

ActionId HeuristicBot::dealer_pickup_discard_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    auto& t = euchre::tables::tables();
    uint32_t h = obs.hand.value();
    Suit trump = obs.trump;

    // Find the weakest card to discard
    // Use trump as led_suit so trump cards get high power and we discard off-suit trash
    Card weakest{};
    uint8_t weakest_power = 255;

    uint32_t remaining = h;
    while (remaining) {
        uint32_t bit = remaining & static_cast<uint32_t>(-static_cast<int32_t>(remaining));
        int idx = std::countr_zero(bit);
        Card c{static_cast<uint8_t>(idx)};

        // Evaluate with trump as led_suit to get meaningful ordering
        // Off-suit cards get low power (0-5), led-suit gets 100+, trump gets 194+
        uint8_t power = t.power[trump][trump][c];
        if (power < weakest_power) {
            weakest_power = power;
            weakest = c;
        }
        remaining &= remaining - 1;
    }

    return euchre::action::discard(weakest);
}

ActionId HeuristicBot::play_trick(const Observation& obs, ActionMask action_mask) {
    auto& t = euchre::tables::tables();
    uint32_t valid_cards = static_cast<uint32_t>(action_mask & 0xFFFFFF); // play actions are [0-24)
    Suit trump = obs.trump;

    if (obs.num_played == 0) {
        // ---- LEADING ----
        uint32_t h = obs.hand.value();
        uint32_t trump_cards = h & t.suit_mask_tbl[trump][trump];
        bool is_maker_team = (obs.player % 2) == obs.maker_team;

        // If maker team with high trump, lead trump to strip opponents
        if (is_maker_team && trump_cards != 0) {
            Card right_bower{trump, Rank::RJ};
            Card left_bower_card{Card{}.same_color(trump), Rank::RJ};
            bool has_high = (h & (1u << right_bower.v)) != 0 ||
                            (h & (1u << left_bower_card.v)) != 0;
            if (has_high) {
                // Lead highest trump
                return euchre::action::play(highest_card(trump_cards, trump, trump));
            }
        }

        // Lead off-suit aces
        for (uint8_t s = 0; s < 4; s++) {
            Suit suit = Suit(s);
            if (suit == trump) continue;
            uint32_t suit_cards = h & t.suit_mask_tbl[trump][suit];
            if (suit_cards == 0) continue;
            Card ace{suit, Rank::RA};
            // Make sure ace isn't the left bower
            if (t.eff_suit_tbl[trump][ace] == trump) continue;
            if (h & (1u << ace.v)) {
                return euchre::action::play(ace);
            }
        }

        // Lead low from shortest off-suit to create voids
        int shortest_len = 7;
        Card best_lead{};
        bool found = false;
        for (uint8_t s = 0; s < 4; s++) {
            Suit suit = Suit(s);
            if (suit == trump) continue;
            uint32_t suit_cards = h & t.suit_mask_tbl[trump][suit];
            if (suit_cards == 0) continue;
            int len = std::popcount(suit_cards);
            if (len < shortest_len) {
                shortest_len = len;
                best_lead = lowest_card(suit_cards, trump, suit);
                found = true;
            }
        }
        if (found) {
            return euchre::action::play(best_lead);
        }

        // Only trump left, lead lowest
        return euchre::action::play(lowest_card(valid_cards, trump, trump));
    }

    // ---- FOLLOWING ----
    Card lead_card = obs.lead;
    Suit led_suit = t.eff_suit_tbl[trump][lead_card];

    // Find current trick winner
    // We need to figure out lead_player. The lead player is the one who played first.
    // Since we know num_played cards have been played and trick_cards has entries,
    // and players go in order from lead_player, we can determine the winner.
    // We need lead_player to call find_trick_winner. We can find it by looking at
    // which entries in trick_cards are valid (not invalid_card).
    // Actually, let's find the lead player by working backwards from our position.
    // We are the (num_played+1)th player to act. But going-alone complicates this.
    // A simpler approach: just find the highest power card among the played trick_cards.
    uint8_t best_power_in_trick = 0;
    uint8_t winner_player = 0;
    for (uint8_t i = 0; i < 4; i++) {
        if (obs.trick_cards[i].v == euchre::constants::invalid_card) continue;
        uint8_t power = t.power[trump][led_suit][obs.trick_cards[i]];
        if (power > best_power_in_trick) {
            best_power_in_trick = power;
            winner_player = i;
        }
    }

    bool partner_winning = (winner_player % 2) == (obs.player % 2);

    if (partner_winning) {
        // Play lowest legal card
        return euchre::action::play(lowest_card(valid_cards, trump, led_suit));
    }

    // Opponent winning — try to beat them cheaply
    Card winner = cheapest_winner(valid_cards, trump, led_suit, best_power_in_trick);
    if (winner.v != euchre::constants::invalid_card) {
        return euchre::action::play(winner);
    }

    // Can't beat — play lowest
    return euchre::action::play(lowest_card(valid_cards, trump, led_suit));
}
