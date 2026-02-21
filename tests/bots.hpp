#include "Phase.hpp"
#include <bots/ScriptedBot.hpp>

namespace test_bots {
    inline auto always_pass = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        return euchre::action::Pass;
    };

    inline auto always_order_up = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        return euchre::action::OrderUp;
    };

    inline auto first_legal = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        return ActionId{static_cast<uint16_t>(std::countr_zero(action_mask))};
    };
  
    inline auto order_up_then_default = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        switch(obs.phase) {
            case Phase::BidRound1:
                return euchre::action::OrderUp;
            default:
                return test_bots::first_legal(obs, action_mask);
        }
    };

    inline auto pass_then_default = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        switch(obs.phase) {
            case Phase::BidRound1:
                return euchre::action::Pass;
            default:
                return test_bots::first_legal(obs, action_mask);
        }
    };

    inline auto always_pass_both_rounds = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) -> ActionId {
        if ((obs.phase == Phase::BidRound1 || obs.phase == Phase::BidRound2)
            && (action_mask & euchre::action::a2m(euchre::action::Pass))) {
            return euchre::action::Pass;
        }
        return test_bots::first_legal(obs, action_mask);
    };

    inline auto call_trump_then_default = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) -> ActionId {
        if (obs.phase == Phase::BidRound2) {
            // Pick the first legal call (skip pass)
            ActionMask calls_only = action_mask & ~euchre::action::a2m(euchre::action::Pass);
            return ActionId{static_cast<uint16_t>(std::countr_zero(calls_only))};
        }
        if (obs.phase == Phase::BidRound1) return euchre::action::Pass;
        return test_bots::first_legal(obs, action_mask);
    };

    inline auto order_up_no_alone = []([[maybe_unused]] const Observation& obs, [[maybe_unused]] ActionMask action_mask) {
        switch(obs.phase) {
            case Phase::BidRound1:
                return euchre::action::OrderUp;
            case Phase::GoAloneDecision:
                return euchre::action::GoAloneNo;
            default:
                return test_bots::first_legal(obs, action_mask);
        }
    };
    

};