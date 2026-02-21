#pragma once

#include <string>
#include "Action.hpp"
#include "Observation.hpp"

using euchre::action::ActionId;
using euchre::action::ActionMask;
class IBot {
    public:

    IBot(std::string name) : m_name(name) {}
    virtual void on_new_match([[maybe_unused]] uint32_t seed) {};
    virtual void on_new_hand([[maybe_unused ]] int hand_index) {};
    virtual ActionId select_action(const Observation& obs, ActionMask action_mask); 
    virtual ~IBot() = default;
    
    protected:

    virtual ActionId bid_phase_1_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) = 0;
    virtual ActionId bid_phase_2_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) = 0;
    virtual ActionId go_alone_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) = 0;
    virtual ActionId dealer_pickup_discard_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) = 0;
    virtual ActionId play_trick(const Observation& obs, [[maybe_unused]] ActionMask action_mask) = 0;

    std::string m_name {};
};