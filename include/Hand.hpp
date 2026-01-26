#pragma once

#include "Tables.hpp"
#include <bit>
#include <iostream>

//using Hand = uint32_t;

struct Hand {

    uint32_t h = 0;
    constexpr Hand() = default;
    constexpr Hand(uint32_t val) : h(val) {};
    constexpr Hand(const Hand& hand) noexcept = default;
    constexpr uint32_t value() const noexcept { return h; }
    constexpr Hand& operator=(const Hand&) noexcept = default;

    constexpr bool empty() { return h == 0; }

    static constexpr Hand from_card(const Card c) {
        return Hand{1u << c.v};
    }

    constexpr void give_card(Card c) {
        assert(c.v < 24);
        h |= (1u << c.v);
    }

    constexpr void give_card(Suit s, Rank r) {
        give_card({s, r});
    }

    uint32_t get_valid_hand(Card led, Suit trump) const {
        euchre::tables::Tables tables = euchre::tables::tables();

        Suit effective_led_suit = tables.eff_suit_tbl[trump][led];
        uint32_t follow = h & tables.suit_mask_tbl[trump][effective_led_suit];

        // If we don't have any ability to follow suit, then we can play anything in our hand.
        return follow ? follow : h;
    }

    bool hand_has(Card c) {
        return ((1u << c.v) & h) > 0;
    }

    inline constexpr void show_hand() {
        uint32_t hand = h;
        while(hand) {
            uint32_t bit = hand & -hand;
            int idx = std::countr_zero<uint32_t>(bit);
            //assert((hand & ~euchre::constants::deck_reset) == 0 && "hand has bits outside 0..23");

            assert(idx < 24);
            Card c(static_cast<uint8_t>(idx));
            std::cout << c << "\t";
            hand &= (hand - 1);
        }

        std::cout << std::endl;
    }

    constexpr int num_cards(Hand hand) {
        return std::popcount(hand.h);
    }
};










