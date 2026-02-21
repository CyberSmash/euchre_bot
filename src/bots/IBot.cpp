#include "bots/IBot.hpp"

ActionId IBot::select_action(const Observation& obs, ActionMask action_mask) {
     switch(obs.phase) {
            case Phase::BidRound1:
                return bid_phase_1_action(obs, action_mask);
            case Phase::BidRound2:
                return bid_phase_2_action(obs, action_mask);
            case Phase::GoAloneDecision:
                return go_alone_action(obs, action_mask);
            case Phase::DealerPickupDiscard:
                return dealer_pickup_discard_action(obs, action_mask);
            case Phase::PlayTrick:
                return play_trick(obs, action_mask);
            default:
                throw std::logic_error("Error: Invalid phase.");
        }
}

