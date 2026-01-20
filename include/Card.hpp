#pragma once
#include "Defns.hpp"
#include <cstdint>
#include <array>
#include <ostream>
#include <cassert>

//using Card = uint8_t;

enum Suit : uint8_t {
    C=0,
    H,
    S,
    D
};


enum Rank : uint8_t {
    R9 = 0,
    RT,
    RJ,
    RQ,
    RK,
    RA
};



struct Card {
    uint8_t v = euchre::constants::invalid_card;

    constexpr Card() = default;
    constexpr Card(uint8_t c) : v(c) {};
    constexpr Card(uint32_t c) : v(static_cast<uint8_t>(c)) {
        assert(c < euchre::constants::num_cards);    
    }
    constexpr Card(int32_t c) : v(static_cast<uint8_t>(c)) {
        assert(c < euchre::constants::num_cards);
    }
    constexpr Card(Suit s, Rank r) {
        v = s*6 + r;
        assert(v < euchre::constants::num_cards);
    }

    constexpr operator uint8_t() const {
        return v;
    }

    ~Card() = default;

};


constexpr std::array<const char*, 4> SUIT_NAMES {
    "Clubs", "Hearts", "Spades", "Diamonds"
};

constexpr std::array<const char*, 4> SUIT_NAMES_SHORT {
    "C", "H", "S", "D"
};

constexpr std::array<const char*, 4> SUIT_SYMBOLS {
    "♣", "♥", "♠", "♦"
};


constexpr std::array<const char*, 6> RANK_NAMES {
    "9", "10", "Jack", "Queen", "King", "Ace"
};

constexpr std::array<const char*, 6> RANK_NAMES_SHORT {
    "9", "10", "J", "Q", "K", "A"
};


constexpr Card make_card(const Suit s, const Rank r) {
    return Card{s, r};
}

constexpr Suit get_suit(Card c) {
    return static_cast<Suit>(c / 6);
}

constexpr Rank get_rank(Card c) {
    return static_cast<Rank>(c % 6);
}

#define SHOW_SUIT_SYMBOL 1
#ifdef SHOW_SUIT_FULL
inline std::ostream& operator<<(std::ostream& os, const Suit s) {
    return os << SUIT_NAMES[s];
}
#elif SHOW_SUIT_SHORT
inline std::ostream& operator<<(std::ostream& os, const Suit s) {
    return os << SUIT_NAMES_SHORT[s];
}
#elif SHOW_SUIT_SYMBOL
inline std::ostream& operator<<(std::ostream& os, const Suit s) {
    return os << SUIT_SYMBOLS[s];
}
#endif

#ifdef SHOW_RANK_FULL
inline std::ostream& operator<<(std::ostream& os, const Rank r) {
    return os << RANK_NAMES[r];
}
#else
inline std::ostream& operator<<(std::ostream& os, const Rank r) {
    return os << RANK_NAMES_SHORT[r];
}
#endif

#ifdef SHOW_FULL_CARD_NAME
inline std::ostream& operator<<(std::ostream& os, const Card& c) {
    return os << get_rank(c) << get_suit(c);
}
#else
inline std::ostream& operator<<(std::ostream& os, const Card& c) {
    return os << get_rank(c) << get_suit(c);
}
#endif

constexpr Suit same_color(Suit s) {
    switch(s) {
        case Suit::C:
            return Suit::S;
        case Suit::S:
            return Suit::C;
        case Suit::H:
            return Suit::D;
        case Suit::D:
            return Suit::H;
    };
}

constexpr bool is_left_bower(Card c, Suit trump) {
    return (get_rank(c) == Rank::RJ && get_suit(c) == same_color(trump));
}

constexpr bool is_right_bower(Card c, Suit trump) {
    return (get_rank(c) == Rank::RJ && get_suit(c) == trump);
}
