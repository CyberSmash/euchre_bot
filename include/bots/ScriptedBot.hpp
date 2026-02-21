#pragma once

#include <functional>
#include "IBot.hpp"

class ScriptedBot : public IBot {
    public:

    using IBot::IBot;
    std::function<ActionId(const Observation, ActionMask)> fn;

    ActionId select_action(const Observation &obs,  ActionMask action_mask) override {
        return fn(obs, action_mask);
    }

    protected:

    virtual ActionId bid_phase_1_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) override {
        throw std::logic_error("Unreachable");
    };
    
    virtual ActionId bid_phase_2_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) override {
        throw std::logic_error("Unreachable");
    };

    virtual ActionId go_alone_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) override {
        throw std::logic_error("Unreachable");
    };

    virtual ActionId dealer_pickup_discard_action([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) override {
        throw std::logic_error("Unreachable");
    };

    virtual ActionId play_trick([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) override {
        throw std::logic_error("Unreachable");
    };
};