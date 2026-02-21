#include "bots/MinBot.hpp"
#include "bots/BotUtils.hpp"
#include <bit>

using euchre::action::ActionId;
using euchre::action::ActionMask;

ActionId MinBot::bid_phase_1_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    return euchre::action::Pass;
}

ActionId MinBot::bid_phase_2_action([[maybe_unused]] const Observation& obs, ActionMask action_mask) {
    // Call first legal trump so games don't stall on redeals
    ActionMask calls_only = action_mask & ~euchre::action::a2m(euchre::action::Pass);
    if (calls_only) {
        return ActionId{static_cast<uint16_t>(std::countr_zero(calls_only))};
    }
    return ActionId{static_cast<uint16_t>(std::countr_zero(action_mask))};
}

ActionId MinBot::go_alone_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    return euchre::action::GoAloneNo;
}

ActionId MinBot::dealer_pickup_discard_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    // Discard lowest card (use trump as led_suit for consistent ordering)
    return euchre::action::discard(bot_utils::lowest_card(obs.hand.value(), obs.trump, obs.trump));
}

ActionId MinBot::play_trick(const Observation& obs, ActionMask action_mask) {
    uint32_t valid_cards = static_cast<uint32_t>(action_mask & 0xFFFFFF);
    Suit led_suit = bot_utils::led_suit_from_obs(obs);
    return euchre::action::play(bot_utils::lowest_card(valid_cards, obs.trump, led_suit));
}
