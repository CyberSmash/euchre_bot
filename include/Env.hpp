#include "Action.hpp"
#include "Defns.hpp"
#include "GameState.hpp"
#include <random>
#include "Deck.hpp"
#include "Observation.hpp"
#include "bots/IBot.hpp"
#include "Phase.hpp"

class Env {

    public:
    Env(unsigned int seed, std::array<IBot*, 4> players) : state(), players(players) {
        state.eng.seed(seed);
    }

    /**
     * @brief Request a legal action from each player.
     *
     * This function exists because we want a single place where we can validate that the returned action 
     * is a legal action.
     * 
     * @param player The player index
     * @param action_mask The legal action mask
     * @return ActionId A action.
     * @throws std::invalid_argument when the player/bot returns an illegal action.
     */
    ActionId request_action(uint8_t player, ActionMask action_mask) {
        Observation obs = state.hand_state.generate_observation(player, state.dealer);
        ActionId action_id = players[player]->select_action(obs, action_mask);
        if ((euchre::action::a2m(action_id) & action_mask) == 0) {
            throw std::invalid_argument("Returned action_id did not match the action mask");
        }
        return action_id;
    }

    /**
     * @brief Deal a hand to each player.
     */
    void deal() {
        // Deal cards to all players.
        for (uint8_t i = 0; i < euchre::constants::num_players; i++) {
            for (uint8_t j = 0; j < 5; j++) {
                Card c = draw_card(state.hand_state.deck, state.eng);
                state.hand_state.give_card_to(c, i);
            }
        }
        // Set the face up card
        state.hand_state.face_up_card = draw_card(state.hand_state.deck, state.eng);
        state.hand_state.phase = Phase::BidRound1;
    }


    uint8_t get_next_player(uint8_t current_player) {

        uint8_t next_player = (current_player + 1) % euchre::constants::num_players;
        // If going alone, we may need to skip a player. 
        if (state.hand_state.going_alone && (next_player == (state.hand_state.maker_player + 2) % euchre::constants::num_players)) {
            next_player = (next_player + 1) % euchre::constants::num_players;
        }

        return next_player;
    }

    /**
     * @brief Goes through the first euchre bidding round.
     * 
     * Each player has the opportunity to tell the dealer to pick up the card, and go alone. 
     */
    void bid_round_1() {
        uint8_t current_player = state.dealer;
        ActionMask action_mask = static_cast<ActionMask>((1ULL << euchre::action::Pass.v | 1ULL << euchre::action::OrderUp.v));
        
        do {
            current_player = (current_player + 1) % euchre::constants::num_players;
            ActionId action_id = request_action(current_player, action_mask);

            if (action_id == euchre::action::OrderUp) {
                state.hand_state.trump = state.hand_state.face_up_card.get_suit();
                state.hand_state.maker_team = current_player % 2;
                state.hand_state.maker_player = current_player;
                state.hand_state.phase = Phase::GoAloneDecision;
                return;
            }

        } while (current_player != state.dealer);
        state.hand_state.phase = Phase::BidRound2;
    }


    void dealer_pickup_discard() {
        state.hand_state.hands[state.dealer].give_card(state.hand_state.face_up_card);
        ActionMask action_mask = static_cast<ActionMask>(state.hand_state.hands[state.dealer].value()) << euchre::constants::num_cards;
        ActionId action = request_action(state.dealer, action_mask);
        Card c {static_cast<uint8_t>(action.v - euchre::constants::num_cards)};
        state.hand_state.hands[state.dealer].remove_card(c);
        state.hand_state.phase = Phase::PlayTrick;
    }

    void go_alone_decision() {
        ActionMask action_mask = euchre::action::make_mask(euchre::action::GoAloneYes, euchre::action::GoAloneNo);
        ActionId action = request_action(state.hand_state.maker_player, action_mask);
        if (action == euchre::action::GoAloneYes) {
            state.hand_state.going_alone = true;
        }
        state.hand_state.phase = Phase::DealerPickupDiscard;
    }

    void bid_round_2() {

        ActionMask action_mask = euchre::action::make_mask(
            euchre::action::Pass, 
            euchre::action::call_trump(Suit::C),
            euchre::action::call_trump(Suit::H),
            euchre::action::call_trump(Suit::S),
            euchre::action::call_trump(Suit::D)

        ) & ~euchre::action::a2m(euchre::action::call_trump(state.hand_state.face_up_card.get_suit()));

        uint8_t current_player = state.dealer;

        do {
            current_player = (current_player + 1) % euchre::constants::num_players;
            
            
            if (current_player == state.dealer && state.hand_state.stick_the_dealer) {
                action_mask &= ~euchre::action::a2m(euchre::action::Pass);
            }

            ActionId action = request_action(current_player, action_mask);
            

            if (action != euchre::action::Pass) {
                state.hand_state.trump = Suit(action.v - euchre::action::CallTrumpBase.v);
                state.hand_state.maker_player = current_player;
                state.hand_state.maker_team = current_player % 2;
                state.hand_state.phase = Phase::GoAloneDecision;
                return;
            }
        } while (current_player != state.dealer);

        // Everyone passed. Redeal.
        state.hand_state.reset();
    }

    void request_tricks() {
         bool first_card = true;
         uint8_t current_player = state.hand_state.lead_player;
      
        do {
            ActionMask action_mask {};
            if (first_card) {
                action_mask = static_cast<ActionMask>(state.hand_state.hands[current_player].value());
            }
            else {
                action_mask = state.hand_state.hands[current_player].get_valid_hand(state.hand_state.lead_card, state.hand_state.trump);
            }

            ActionId action = request_action(current_player, action_mask);
            Card c {static_cast<uint8_t>(action.v)};
            state.hand_state.hands[current_player].remove_card(c);
            if (first_card) {
                // In this case we need to set the lead card.
                state.hand_state.lead_card = c;
                first_card = false;
            }
            // Set the trick cards, increment the count.
            state.hand_state.trick_cards[current_player] = c;
            state.hand_state.num_played++;
            current_player = get_next_player(current_player);

        } while(current_player != state.hand_state.lead_player);
    }

    void calc_winner() {
        auto& t = euchre::tables::tables();
        Suit led_suit = t.eff_suit_tbl[state.hand_state.trump][state.hand_state.lead_card];
        uint8_t winner = 0;
        uint8_t best_power = 0;
        Suit trump = state.hand_state.trump;
        for (uint8_t i = 0; i < euchre::constants::num_players; i++) {
            // Skip the entry for the skipped partner.
            if (state.hand_state.going_alone && i == (state.hand_state.maker_player + 2 ) % euchre::constants::num_players) {
                continue;
            }
            
            uint8_t power = t.power[trump][led_suit][state.hand_state.trick_cards[i]];
            if (power > best_power) {
                best_power = power;
                winner = i;
            }

        }

        state.hand_state.tricks_won[winner % 2]++;
        state.hand_state.lead_player = winner;
    }

    void play_trick() {

        if (state.hand_state.tricks_played == 0) {
            state.hand_state.lead_player = get_next_player(state.dealer);
        }
        request_tricks();
        calc_winner();
        state.hand_state.tricks_played++;
        state.hand_state.lead_card = euchre::constants::invalid_card;
        

        if (state.hand_state.tricks_played == 5) {
            state.hand_state.phase = Phase::HandOver;
        }

        state.hand_state.num_played = 0;
        
    }

    void hand_over() {
        
        uint8_t maker_team = state.hand_state.maker_team;
        uint8_t maker_tricks = state.hand_state.tricks_won[state.hand_state.maker_team];
        uint8_t defender_team = 1 - state.hand_state.maker_team;

        // Score
        if (maker_tricks == 5) {
            state.scores[maker_team] += state.hand_state.going_alone ? 4 : 2;
        }
        else if (maker_tricks >= 3) {
            state.scores[maker_team] += 1;
        }
        else {
            state.scores[defender_team] += 2;
        }

        // Rotate dealer
        state.dealer = (state.dealer + 1) % euchre::constants::num_players;

        // Reset hand state
        state.hand_state.reset();

    }

    void step_hand() {
        switch(state.hand_state.phase) {
            case Phase::Deal:
                deal();
                break;
            case Phase::BidRound1:
                bid_round_1();
                break;
            case Phase::GoAloneDecision:
                go_alone_decision();
                break;
            case Phase::DealerPickupDiscard:
                dealer_pickup_discard();
                break;
            case Phase::BidRound2:
                bid_round_2();
                break;
            case Phase::PlayTrick:
                play_trick();
                break;
            case Phase::HandOver:
                hand_over();
                break;
            default:
                throw std::logic_error("State not implemented");
        }
    }

    void step_game() {
        step_hand();
        if (state.scores[0] >= 10 || state.scores[1] >= 10) {
            state.status = GameState::GameStatus::GameOver;
        }
    }

    GameState state;
    std::array<IBot*, 4> players;

};