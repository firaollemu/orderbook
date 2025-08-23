#pragma once
#include "strategy.hpp"


// BaseLineMarketMaking inherits from StrategyInterface; it is an implementation of it
struct BaseLineMarketMaking : public StrategyInterface {
    int spread = 2;
    int skew_per_unit = 1;
    int qty = 5;

    std::vector<QuoteAction> step(const EngineAPI& api) override {
        auto best_bid = api.ob.best_bid();
        auto best_ask = api.ob.best_ask();
        // when no best bid and best ask, fall back to 100
        // assume 100 is the mid price
        // mid -> avg of best bid and best ask
        // spread difference between 
        Px mid = (best_bid && best_ask) ? ((*best_bid + *best_ask)/2) : (best_bid ? *best_bid : (best_ask ? *best_ask : 100));


        int skew = static_cast<int>(api.pnl.inventory) * skew_per_unit; 
        Px bid = mid - spread/2 - skew;
        Px ask = mid + spread/2- skew;

        // Quote -> always return a pair of bid and ask prices
        return {
            {Side::Buy, bid, qty},
            {Side::Sell, ask, qty}
        };
    }
};