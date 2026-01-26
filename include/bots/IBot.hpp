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
    virtual ActionId select_action(const Observation& obs, ActionId action, ActionMask action_mask) = 0; 
    virtual ~IBot() = default;
    
    protected:
    std::string m_name {};
};