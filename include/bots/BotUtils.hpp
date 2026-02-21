#pragma once

#include "Card.hpp"
#include "Hand.hpp"
#include "Observation.hpp"
#include "Tables.hpp"
#include <bit>

namespace bot_utils {

struct HandStrength {
    int trump_count;
    bool has_right;
    bool has_left;
    int off_aces;
    int score;
};

inline HandStrength evaluate_hand(Hand hand, Suit trump) {
    auto& t = euchre::tables::tables();
    uint32_t h = hand.value();

    uint32_t trump_cards = h & t.suit_mask_tbl[trump][trump];
    int trump_count = std::popcount(trump_cards);

    Card right_bower{trump, Rank::RJ};
    Card left_bower_card{Card{}.same_color(trump), Rank::RJ};
    bool has_right = (h & (1u << right_bower.v)) != 0;
    bool has_left = (h & (1u << left_bower_card.v)) != 0;

    int off_aces = 0;
    for (uint8_t s = 0; s < 4; s++) {
        if (Suit(s) == trump) continue;
        Card ace{Suit(s), Rank::RA};
        // Left bower ace doesn't count as off-ace — check effective suit
        if (t.eff_suit_tbl[trump][ace] != trump && (h & (1u << ace.v)) != 0) {
            off_aces++;
        }
    }

    int score = trump_count + (has_right ? 2 : 0) + (has_left ? 1 : 0) + off_aces;
    return {trump_count, has_right, has_left, off_aces, score};
}

// Get the lowest-power card from a bitmask of cards
inline Card lowest_card(uint32_t cards, Suit trump, Suit led_suit) {
    auto& t = euchre::tables::tables();
    Card best{};
    uint8_t best_power = 255;

    uint32_t remaining = cards;
    while (remaining) {
        uint32_t bit = remaining & static_cast<uint32_t>(-static_cast<int32_t>(remaining));
        int idx = std::countr_zero(bit);
        Card c{static_cast<uint8_t>(idx)};
        uint8_t power = t.power[trump][led_suit][c];
        if (power < best_power) {
            best_power = power;
            best = c;
        }
        remaining &= remaining - 1;
    }
    return best;
}

// Get the highest-power card from a bitmask of cards
inline Card highest_card(uint32_t cards, Suit trump, Suit led_suit) {
    auto& t = euchre::tables::tables();
    Card best{};
    uint8_t best_power = 0;
    bool found = false;

    uint32_t remaining = cards;
    while (remaining) {
        uint32_t bit = remaining & static_cast<uint32_t>(-static_cast<int32_t>(remaining));
        int idx = std::countr_zero(bit);
        Card c{static_cast<uint8_t>(idx)};
        uint8_t power = t.power[trump][led_suit][c];
        if (!found || power > best_power) {
            best_power = power;
            best = c;
            found = true;
        }
        remaining &= remaining - 1;
    }
    return best;
}

// Get cheapest card that beats a given power level, or invalid if none can
inline Card cheapest_winner(uint32_t cards, Suit trump, Suit led_suit, uint8_t beat_power) {
    auto& t = euchre::tables::tables();
    Card best{};
    uint8_t best_power = 255;

    uint32_t remaining = cards;
    while (remaining) {
        uint32_t bit = remaining & static_cast<uint32_t>(-static_cast<int32_t>(remaining));
        int idx = std::countr_zero(bit);
        Card c{static_cast<uint8_t>(idx)};
        uint8_t power = t.power[trump][led_suit][c];
        if (power > beat_power && power < best_power) {
            best_power = power;
            best = c;
        }
        remaining &= remaining - 1;
    }
    return best;
}

// Get effective led suit from an observation
inline Suit led_suit_from_obs(const Observation& obs) {
    auto& t = euchre::tables::tables();
    if (obs.num_played == 0) {
        return obs.trump; // Leading: use trump for consistent power ordering
    }
    return t.eff_suit_tbl[obs.trump][obs.lead];
}

} // namespace bot_utils
