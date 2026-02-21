#pragma once

#include <cstdint>

enum class Phase : uint8_t { 
    Deal,
    BidRound1,
    DealerPickupDiscard,
    GoAloneDecision,
    BidRound2,
    PlayTrick,
    HandOver,

};