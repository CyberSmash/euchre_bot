#pragma once

#include "IBot.hpp"

class MinBot : public IBot {
public:
    MinBot(std::string name) : IBot(std::move(name)) {}

protected:
    ActionId bid_phase_1_action(const Observation& obs, ActionMask action_mask) override;
    ActionId bid_phase_2_action(const Observation& obs, ActionMask action_mask) override;
    ActionId go_alone_action(const Observation& obs, ActionMask action_mask) override;
    ActionId dealer_pickup_discard_action(const Observation& obs, ActionMask action_mask) override;
    ActionId play_trick(const Observation& obs, ActionMask action_mask) override;
};
