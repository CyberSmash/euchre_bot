#pragma once
#include "Card.hpp"
#include "Defns.hpp"

namespace euchre::tables {
    using SuitTable = std::array<Suit, euchre::constants::num_cards>;
    using SuitMaskTable = std::array<std::array<uint32_t, 4>, 4>;
    using EffSuitTable = std::array<std::array<Suit, 24>, 4>;
    using PowerTable = std::array<std::array<std::array<uint8_t, 24>, 4>, 4>;
    
    struct Tables {
        SuitTable suit_tbl {};
        SuitMaskTable suit_mask_tbl {};
        EffSuitTable eff_suit_tbl {};
        PowerTable power {};
        uint32_t deck;
    };




    constexpr SuitTable make_suit_table() {
        SuitTable t{};
        for (std::size_t i = 0; i < euchre::constants::num_cards; i++) {
            t[i] = static_cast<Suit>(i / 6);
        }
        return t;
    }

    constexpr EffSuitTable make_eff_suit_table() {
        EffSuitTable t{};
        for (uint8_t tr = 0; tr < 4; tr++) {
            Suit trump = Suit(tr);
            for (uint32_t c = 0; c < 24; c++) {
                if (is_left_bower(c, trump)) {
                    t[trump][c] = trump;
                    continue;
                }

                t[trump][c] = get_suit(c);

            }
        }
        return t;

    }

    constexpr uint8_t get_trump_power(Card c, Suit trump) {
        uint8_t power = 0;
        if (is_right_bower(c, trump)) {
            power = 200;
        }
        else if (is_left_bower(c, trump)) {
            power = 199;
        }
        else {
            Rank rank = get_rank(c);
            switch(rank) {
                case Rank::RA:
                    power = 198;
                    break;
                case Rank::RK:
                    power = 197;
                    break;
                case Rank::RQ:
                    power = 196;
                    break;
                case Rank::RT:
                    power = 195;
                    break;
                case Rank::R9:
                    power = 194;
                    break;
                default:
                    power = 0;
                    break;
            }
        }
        return power;
    }

    /**
    * Create a lookup table, that determines the power of each card. 
    */
    constexpr void make_power_table(Tables& t) {
        auto base_rank_power = [](Rank r) -> uint8_t {
            return static_cast<uint8_t>(r);
        };

        for(uint8_t tr = 0; tr < 4; tr++) {
            Suit trump = Suit(tr);

            for (uint8_t led = 0; led < 4; led++) {
                Suit led_suit = Suit(led);

                for(uint8_t i = 0; i < 24; i++) {
                    Card c{i};
                    Suit eff_card_suit = t.eff_suit_tbl[trump][c];
                    uint8_t power = 0;

                    if (eff_card_suit == trump) {

                        power = get_trump_power(c, trump);
                    }

                    else if (eff_card_suit == led_suit) {
                        Rank r = get_rank(c);
                        power = static_cast<uint8_t>(100 + base_rank_power(r));
                    }
                    else {
                        Rank r = get_rank(c);
                        power = static_cast<uint8_t>(base_rank_power(r));
                    }

                    t.power[tr][led][c] = power;
                }
            } 
        }
    }

    static constexpr auto build_suit_mask(const EffSuitTable effective_suit_tbl) {
        std::array<std::array<uint32_t, 4>, 4> suit_mask{}; // [trump][suit]

        for (uint8_t t = 0; t < 4; ++t) {
            for (uint8_t s = 0; s < 4; ++s) {
                uint32_t m = 0;
                for (uint8_t c = 0; c < 24; ++c) {
                    if (effective_suit_tbl[t][c] == Suit{s}) {
                        m |= (uint32_t{1} << c);
                    }
                }
                suit_mask[t][s] = m;
            }
        }
        return suit_mask;
    }

    consteval Tables make_tables() {
        Tables t;
        t.suit_tbl = make_suit_table();
        t.eff_suit_tbl = make_eff_suit_table();
        t.deck = (1 << euchre::constants::num_cards) - 1;
        t.suit_mask_tbl = build_suit_mask(t.eff_suit_tbl);
        make_power_table(t);

        return t;
    }

    inline const Tables& tables() {
        static const Tables t = make_tables();
        return t;
    }
};

