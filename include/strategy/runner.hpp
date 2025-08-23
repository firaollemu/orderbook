#pragma once
#include "strategy.hpp"
#include "order_utils.hpp"

// runner -> applies actions returned by strategies

using orderutil::mk_order;

inline void apply_actions(const std::vector<QuoteAction>& actions, EngineAPI& api) {
    for (const auto& action : actions) {
        api.ob.submit_limit(mk_order(kInvalidOrderId, action.side, action.px, action.qty, api.ts));
    }
}