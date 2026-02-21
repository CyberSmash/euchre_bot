# Euchre Engine

A high-performance Euchre card game engine written in modern C++20, designed for eventual self-play neural network training via PyTorch. Features bitwise card/hand representations, compile-time lookup tables, and a flat action space suitable for policy network output masking.

## Features

- **Complete Euchre Rules** -- Full game loop: dealing, two-round bidding, going alone, trick-taking, scoring, and game-over detection
- **NN-Friendly Action Space** -- 56 flat actions `[0,56)` with bitmask legal-action masking, ready for policy network integration
- **Bitwise Representations** -- 24-bit deck, 32-bit hands, 64-bit action masks for efficient set operations
- **Compile-Time Lookup Tables** -- `consteval`-generated power, effective suit, and suit mask tables for O(1) card evaluation
- **Stateless Bot Interface** -- Bots are pure functions (observation + legal actions in, action out), decoupled from engine state
- **Reproducible RNG** -- Seeded `std::mt19937` for deterministic testing and replay
- **Configurable Rules** -- Stick-the-dealer toggle, redeal on full pass

## Project Structure

```
include/
    Card.hpp           # Card struct (uint8_t), suit/rank/bower queries
    Hand.hpp           # 32-bit bitmask hand with follow-suit logic
    Deck.hpp           # Card drawing, pick_random_bit<T> template
    Tables.hpp         # consteval power, effective suit, suit mask tables
    Action.hpp         # Flat action encoding [0,56), ActionMask utilities
    Phase.hpp          # Phase enum (Deal, BidRound1, BidRound2, etc.)
    HandState.hpp      # Per-hand state (deck, hands, trump, tricks, etc.)
    GameState.hpp      # Per-game state (scores, dealer, RNG, status)
    Observation.hpp    # Bot's view of the game
    Env.hpp            # Game engine: state machine, bot orchestration
    Defns.hpp          # Constants and type aliases
    bots/
        IBot.hpp       # Abstract bot interface
        RandomBot.hpp  # Picks random legal actions
        ScriptedBot.hpp # Lambda-driven bot for testing
src/
    main.cpp           # Entry point / scratch pad
    Deck.cpp           # draw_card implementation
    Action.cpp         # decode_action implementation
    bots/
        IBot.cpp
        RandomBot.cpp
tests/
    bots.hpp           # Reusable ScriptedBot lambdas for tests
    test_cards.cpp     # Card encoding, bower identification
    test_deck.cpp      # Dealing, reproducibility, no duplicates
    test_tables.cpp    # Effective suit, power hierarchy, bower power
    test_hand.cpp      # Valid plays, follow-suit, left bower rules
    test_action.cpp    # Action encoding/decoding, mask utilities
    test_env.cpp       # Phase transitions, bidding, trick-taking, going alone
    test_game.cpp      # Scoring, dealer rotation, full game integration
```

## Building

### Requirements

- CMake 3.24+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)

### Build Instructions

```bash
# Debug build (sanitizers + warnings-as-errors enabled)
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Disable sanitizers if needed
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=OFF
```

### Running Tests

```bash
ctest --test-dir build --output-on-failure

# List available tests
./build/tests --list-tests

# Run a specific test
./build/tests "Full game terminates"
```

## Quick Example

```cpp
#include "Env.hpp"
#include "bots/RandomBot.hpp"

// Create bots
RandomBot bot_a{"A"}, bot_b{"B"}, bot_c{"C"}, bot_d{"D"};
std::array<IBot*, 4> players = {&bot_a, &bot_b, &bot_c, &bot_d};

// Run a full game
Env env{12345, players};
while (env.state.status != GameState::GameStatus::GameOver) {
    env.step_game();
}
// env.state.scores[0] and env.state.scores[1] have final scores
```

```cpp
#include "Card.hpp"
#include "Tables.hpp"

// Cards are uint8_t values: suit * 6 + rank
Card jack_of_hearts = make_card(Suit::H, Rank::RJ);
jack_of_hearts.is_right_bower(Suit::H);  // true
jack_of_hearts.is_left_bower(Suit::D);   // true (same color as diamonds trump)

// Compile-time power tables for instant trick evaluation
auto& t = euchre::tables::tables();
uint8_t power = t.power[Suit::H][Suit::H][jack_of_hearts];  // 200 (right bower)
```

## Roadmap

- [ ] Heuristic bot for stronger baseline play
- [ ] Observation tensorization for NN input
- [ ] Game history / replay tracking
- [ ] PyTorch C++ integration for self-play training
- [ ] Performance benchmarking and optimization
