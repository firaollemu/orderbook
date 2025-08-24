#pragma once
#include "strategy.hpp"
#include "order_utils.hpp"

// runner -> applies actions returned by strategies

using orderutil::mk_order;

inline void apply_actions(const std::vector<ModAction>& actions, EngineAPI& api) {
    for (const auto& action : actions) {
        switch (action.type) {
            case ModType::New:
                api.ob.submit_limit(mk_order(kInvalidOrderId, action.side, action.px, action.qty, api.ts));
                break;
            case ModType::Cancel:
                api.ob.cancel(action.id);
                break;
            default:
                break;
        }
    }
}