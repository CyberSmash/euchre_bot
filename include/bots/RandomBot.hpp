#include "IBot.hpp"
#include <random>
class RandomBot : public IBot {

    public:
    
        RandomBot(std::string name) : IBot(std::move(name)) {}
    virtual void on_new_match([[maybe_unused]] uint32_t seed) override;
    ActionId select_action(const Observation& obs, [[maybe_unused]]ActionMask action_mask) override;

    protected:

    virtual ActionId bid_phase_1_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) override;
    virtual ActionId bid_phase_2_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) override;
    virtual ActionId go_alone_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) override;
    virtual ActionId dealer_pickup_discard_action(const Observation& obs, [[maybe_unused]] ActionMask action_mask) override;
    virtual ActionId play_trick(const Observation& obs, [[maybe_unused]] ActionMask action_mask) override;
    
    std::mt19937 rng;
};

