# Euchre Engine

A high-performance Euchre card game engine written in modern C++20. This library provides efficient core game logic, card evaluation, and a foundation for AI development and multiplayer game implementations.

## Features

- **Compact Card Representation** — Single-byte encoding for all 24 Euchre cards
- **Zero-Cost Abstractions** — Compile-time generated lookup tables for O(1) card power evaluation
- **Complete Euchre Rules** — Proper handling of bowers, trump suits, and effective suit logic
- **Reproducible RNG** — Seeded random number generation for testing and replay support
- **Modern C++20** — Leverages `consteval`, `<bit>` operations, and strict type safety
- **Thoroughly Tested** — Unit tests covering cards, deck operations, and power tables

## Project Structure

```
├── include/
│   ├── Card.hpp        # Card representation & euchre-specific operations
│   ├── Deck.hpp        # Card drawing interface
│   ├── Defns.hpp       # Game constants (deck size, bitmasks)
│   ├── Env.hpp         # Environment combining RNG + game state
│   ├── GameState.hpp   # Game state tracking
│   ├── Rng.hpp         # Random number generation wrapper
│   └── Tables.hpp      # Pre-computed power & suit lookup tables
├── src/
│   ├── main.cpp        # Demo / entry point
│   ├── euchre.cpp      # Core game logic
│   └── Deck.cpp        # Card dealing implementation
├── tests/
│   ├── test_cards.cpp  # Card encoding/decoding tests
│   ├── test_deck.cpp   # Deck dealing tests
│   └── test_tables.cpp # Lookup table validation
└── CMakeLists.txt
```

## Building

### Requirements

- CMake 3.24+
- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/euchre-engine.git
cd euchre-engine

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the executable
./build/euchre
```

### Running Tests

```bash
# Build with tests
cmake -B build
cmake --build build

# Run all tests
ctest --test-dir build --output-on-failure
```

## Usage

### Card Basics

```cpp
#include "Card.hpp"

// Create cards
Card ace_of_spades = make_card(Suit::S, Rank::A);
Card jack_of_hearts = make_card(Suit::H, Rank::J);

// Query card properties
Suit s = get_suit(ace_of_spades);  // Suit::S
Rank r = get_rank(ace_of_spades);  // Rank::A

// Euchre-specific checks
bool is_right = is_right_bower(jack_of_hearts, Suit::H);  // true (jack of trump)
bool is_left = is_left_bower(jack_of_hearts, Suit::D);    // true (jack of same color as trump)
```

### Card Power Evaluation

The engine uses pre-computed lookup tables to instantly determine card strength:

```cpp
#include "Tables.hpp"

// PowerTable[trump][led_suit][card] returns power value
// Higher values beat lower values

// When hearts is trump and hearts is led:
// Right bower (J♥): 200
// Left bower (J♦):  199
// A♥: 198, K♥: 197, Q♥: 196, 10♥: 195, 9♥: 194

// When spades is led (hearts still trump):
// A♠: 105, K♠: 104, Q♠: 103, J♠: 102, 10♠: 101, 9♠: 100

// Off-suit cards that can't win: 0-5
```

### Deck Operations

```cpp
#include "Deck.hpp"
#include "Defns.hpp"
#include "Rng.hpp"

Rng rng(12345);  // Seeded for reproducibility
uint32_t deck = deck_reset;  // All 24 cards available

// Draw a random card (modifies deck bitmask)
uint8_t card_index = draw_card(deck, rng);
Card card{card_index};
```

### Game State

```cpp
#include "GameState.hpp"

GameState state;
state.deck = deck_reset;
state.current_player = 0;
state.trump = Suit::H;
state.maker_team = 0;  // Team 0 called trump
```

## Card Encoding

Cards are encoded as a single `uint8_t` using the formula: `value = suit × 6 + rank`

| Suit | Code | Ranks (9, 10, J, Q, K, A) |
|------|------|---------------------------|
| Clubs (♣) | 0 | 0-5 |
| Hearts (♥) | 1 | 6-11 |
| Spades (♠) | 2 | 12-17 |
| Diamonds (♦) | 3 | 18-23 |

The deck is represented as a 24-bit bitmask, enabling efficient set operations for tracking remaining cards.

## Euchre Rules Implemented

- **Trump Suit** — Any of the four suits can be trump
- **Right Bower** — Jack of trump suit (highest card)
- **Left Bower** — Jack of same color as trump (second highest)
- **Effective Suit** — Left bower is considered trump suit for following suit
- **Card Hierarchy** — Complete power ranking for trick evaluation

## Development

### Debug Build with Sanitizers

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Debug builds enable:
- AddressSanitizer
- UndefinedBehaviorSanitizer
- Warnings treated as errors

### Code Quality

The project enforces strict compiler warnings including:
- `-Wall -Wextra -Wpedantic`
- `-Wconversion -Wsign-conversion`
- `-Wshadow -Wnull-dereference`
- And many more...

## Roadmap

- [ ] Complete game loop implementation
- [ ] Trick-taking logic
- [ ] Bidding phase
- [ ] AI player strategies
- [ ] Game scoring (points, euchres, loners)
- [ ] Network multiplayer support

## License

[Add your license here]

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.
