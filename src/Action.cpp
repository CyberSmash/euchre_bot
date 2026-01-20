#include "Action.hpp"

namespace euchre::action {

DecodedAction decode_action(ActionId action) {
        uint16_t a = action.v;

        if (is_play(action)) {
            auto c = Card(action);
            return {
                .kind = ActionKind::PlayCard,
                .card = c,
                .suit = get_suit(c)
            };
        }
        else if (is_discard(action)) {
            auto c = Card(action - euchre::constants::num_cards);

            return {
                .kind = ActionKind::DiscardCard,
                .card = c,
                .suit = get_suit(c),
            };
        }
        else if (is_call_trump(action)) {
            return {
                .kind = ActionKind::CallTrump,
                .card = euchre::constants::invalid_card,
                .suit = Suit(a - static_cast<uint16_t>(ActionKind::CallTrump))
            };
        }
        else if (action == OrderUp ) {
            return {
                .kind = ActionKind::OrderUp,
                .card = {},
                .suit = {},
            };
        }
        else if (action == Pass) {
            return {
                .kind = ActionKind::Pass,
                .card = {},
                .suit = {},
            };
        }
        else if (action == GoAloneYes) {
            return {
                .kind = ActionKind::GoAloneYes,
                .card = {},
                .suit = {},
            };
        }
        else if (action == GoAloneNo) {
            return {
                .kind = ActionKind::GoAloneNo,
                .card = {},
                .suit = {}
            };
        }
        return {
            .kind = ActionKind::InvalidAction,
            .card = {},
            .suit = {},
        };
    }
}