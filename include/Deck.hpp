#pragma once
#include <cstdint>
#include <random>
#include "Card.hpp"

Card draw_card(uint32_t& remaining_cards, std::mt19937& rng);