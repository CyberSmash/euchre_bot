#pragma once
#include <cstdint>

namespace euchre::constants {
    inline constexpr int num_cards = 24;
    inline constexpr int deck_reset = (1 << num_cards) - 1;
    inline constexpr uint8_t invalid_card = 0xFF;
    inline constexpr int num_players = 4;
};
