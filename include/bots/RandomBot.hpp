#include "IBot.hpp"
#include "Tables.hpp"
#include <random>
#include "Hand.hpp"
class RandomBot : public IBot {

    virtual void on_new_match([[maybe_unused]] uint32_t seed) override {
        eng.seed(seed);
    };

    virtual ActionId select_action(const Observation& obs, ActionId action, ActionMask action_mask) override {

        Hand valid_hand = 0;
        if (obs.trick_cards == 0) {
            // We are leading and can play whatever.
            valid_hand = obs.hand;
        }
        else {
            valid_hand = obs.hand.get_valid_hand(obs.lead, obs.trump);
        }

    }


    protected:

    std::mt19937 eng;
};

