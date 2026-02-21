#include <bots/RandomBot.hpp>
#include "Deck.hpp"

void RandomBot::on_new_match([[maybe_unused]] uint32_t seed) {
    rng.seed(seed);
};

ActionId RandomBot::select_action([[maybe_unused]] const Observation& obs, [[maybe_unused]]ActionMask action_mask) {
    return ActionId {static_cast<uint16_t>(pick_random_bit(action_mask, rng))};
}

ActionId RandomBot::bid_phase_1_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    throw std::logic_error("Unreachable");
}

ActionId RandomBot::bid_phase_2_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    throw std::logic_error("Unreachable");
}

ActionId RandomBot::go_alone_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    throw std::logic_error("Unreachable");
}

ActionId RandomBot::play_trick([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    throw std::logic_error("Unreachable");
}

ActionId RandomBot::dealer_pickup_discard_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
    throw std::logic_error("Unreachable");
}