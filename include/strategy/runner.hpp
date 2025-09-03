#pragma once

#include <vector>
#include "strategy/strategy.hpp"
#include "order_utils.hpp"
#include "order.hpp"
#include "csv_logger.hpp"
#include "modification_action.hpp"


// runner -> applies actions returned by strategies

using aStrategy::ModificationType;
using orderutil::mk_order;


namespace aStrat {
    struct ApplyResult {
        std::vector<OrderId> placed_ids;
        int passed_cancels{0};
        int failed_cancels{0};
        size_t total_fills{0}; // size_t -> unsigned integer type which can be large enough to hold the size of the largest possible object in memory
    };

    inline ApplyResult apply_modifications(const std::vector<ModificationAction>& actions,
                            EngineAPI& api, bool log_csv = true, bool update_pnl = true 
    ){
        ApplyResult out;

        for (const auto& action : actions) {
            switch (action.type) {
                case ModificationType::Cancel: {
                    const bool ok = api.ob.cancel(action.id);
                    if (ok) {
                        out.passed_cancels++;
                    } else {
                        out.failed_cancels++;
                    }
                    break;
                }
                case ModificationType::New: {
                    // need side, px, qty to put a new quote 
                    if (!action.side || !action.px || !action.qty) break;

                    Order o = mk_order(action.id, *action.side, *action.px, *action.qty, api.ts);
                    auto res = api.ob.submit_limit(o);

                    // res gives us the id and fills

                    if (log_csv && !res.fills.empty()) csvlog::append_fills(res.fills);
                    if (update_pnl) {
                        const Side our_side = action.side.value();
                        for (const auto& fill : res.fills) {
                            // calcuate pnl based on the given fill (a successful execution of an order)
                            api.pnl.on_trade(our_side, fill.px, fill.qty);
                        }
                    }
                    out.total_fills += res.fills.size();
                    out.placed_ids.push_back(res.id);
                    break;
                    
                }
            }
        }
        return out;
    }
    
}