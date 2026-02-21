#pragma once

#include <cstdint>
#include "Card.hpp"
#include "Defns.hpp"

namespace euchre::action {

    using ActionMask = uint64_t;
    


    struct ActionId {
        uint16_t v;
        constexpr ActionId() : v(0) {};
        constexpr explicit ActionId(uint16_t action) : v(action) {}
        constexpr operator uint16_t() const { return v; }

    };

    constexpr ActionMask a2m(ActionId action) {
        return (1ULL << action.v);
    }

    template <typename... Args>
        requires(std::same_as<Args, ActionId> && ...)
    constexpr ActionMask make_mask(Args... actions) {
        return (a2m(actions) | ...);
    }

    static inline constexpr uint16_t num_actions = 56;
    static inline constexpr ActionId PlayCardBase {0}; // 0 - 23
    static inline constexpr ActionId PlayCardEnd {23}; 

    static inline constexpr ActionId DiscardCardBase {euchre::constants::num_cards}; // 24 - 47
    static inline constexpr ActionId DiscardCardEnd {47}; 
    static inline constexpr ActionId Pass {48};
    static inline constexpr ActionId OrderUp {49};
    static inline constexpr ActionId CallTrumpBase {50};
    static inline constexpr ActionId CallTrumpEnd {53};
    static inline constexpr ActionId GoAloneYes {54};
    static inline constexpr ActionId GoAloneNo {55};
    static inline constexpr ActionId InvalidAction {56};
    
    static constexpr ActionId play(Card c) { return ActionId(c.v); };
    static constexpr ActionId discard(Card c) { return ActionId(c.v + constants::num_cards); };
    static constexpr ActionId call_trump(Suit s) { return ActionId(static_cast<uint16_t>(CallTrumpBase.v) + static_cast<uint8_t>(s)); }
    
    static constexpr bool is_play(ActionId action) {
        return action.v >= PlayCardBase && action.v <= PlayCardEnd;
    }

    static constexpr bool is_discard(ActionId action) {
        return action.v >= DiscardCardBase && action.v <= DiscardCardEnd;
    }

    static constexpr bool is_call_trump(ActionId action) {
        return action.v >= CallTrumpBase && action.v <= CallTrumpEnd;
    }

    enum class ActionKind : uint8_t {
        PlayCard,
        DiscardCard,
        OrderUp,
        CallTrump,
        Pass,
        GoAloneYes,
        GoAloneNo,
        InvalidAction,
    };

    struct DecodedAction {
        ActionKind kind;
        Card card;
        Suit suit;
    };

    
    DecodedAction decode_action(ActionId action);


};